#!/usr/bin/env python3
"""Patch ManalinkEh.dll AI decision time to clamp high configured values.

The active duel engine reads AiDecisionTime from config.txt. The fallback patch
keeps missing or invalid values fast; this patch also caps positive configured
values above 270 so a copied install cannot look frozen because of a drifted
high timer setting.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCHED_SECTION_VIRTUAL_SIZE = 0x80
ACCEPTED_SECTION_VIRTUAL_SIZES = (0x30, PATCHED_SECTION_VIRTUAL_SIZE, 0x100)
MAX_AI_DECISION_TIME = 270
NOP = b"\x90"

ACCEPTED_PREIMAGES = {
    bytes.fromhex("89c385c07f05bb1d150000"): 5405,
    bytes.fromhex("89c385c07f05bb1c020000"): 540,
    bytes.fromhex("89c385c07f05bb0e010000"): 270,
}
PATCH_LENGTH = len(next(iter(ACCEPTED_PREIMAGES)))


@dataclass(frozen=True)
class PatchSpec:
    patch_offset: int
    patch_vma: int
    after_patch_vma: int
    cave_offset: int
    cave_vma: int
    section_raw: int
    section_vma: int


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        patch_offset=0x40D0E1,
        patch_vma=0x0240DAE1,
        after_patch_vma=0x0240DAEC,
        cave_offset=0x495A60,
        cave_vma=0x02497060,
        section_raw=0x495A00,
        section_vma=0x00497000,
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x3D2DA1,
        patch_vma=0x023D37A1,
        after_patch_vma=0x023D37AC,
        cave_offset=0x452C60,
        cave_vma=0x02454060,
        section_raw=0x452C00,
        section_vma=0x00454000,
    ),
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def rel32(from_vma_after_instruction: int, to_vma: int) -> bytes:
    return struct.pack("<i", to_vma - from_vma_after_instruction)


def jmp_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe9" + rel32(from_vma + 5, to_vma)


def build_cave(spec: PatchSpec) -> bytes:
    cave = bytearray()
    cave += b"\x89\xc3"  # mov ebx, eax
    cave += b"\x85\xc0"  # test eax, eax
    cave += b"\x7e\x08"  # jle set_default
    cave += b"\x81\xfb" + struct.pack("<I", MAX_AI_DECISION_TIME)
    cave += b"\x7e\x05"  # jle done
    cave += b"\xbb" + struct.pack("<I", MAX_AI_DECISION_TIME)
    cave += jmp_rel32(spec.cave_vma + len(cave), spec.after_patch_vma)
    return bytes(cave)


def patch_jump(spec: PatchSpec) -> bytes:
    return jmp_rel32(spec.patch_vma, spec.cave_vma) + NOP * (PATCH_LENGTH - 5)


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_PATCHES:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_PATCHES:
            return candidate
    raise SystemExit(f"FAIL: no known AI decision clamp site for {path}")


def find_section_virtual_size_offset(data: bytes, spec: PatchSpec) -> int:
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    section_count = struct.unpack_from("<H", data, pe_offset + 6)[0]
    optional_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    section_table = pe_offset + 24 + optional_header_size

    for index in range(section_count):
        section_offset = section_table + 40 * index
        virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from(
            "<IIII", data, section_offset + 8
        )
        if raw_pointer == spec.section_raw and virtual_address == spec.section_vma:
            required_raw = spec.cave_offset - spec.section_raw + PATCHED_SECTION_VIRTUAL_SIZE
            if raw_size < required_raw:
                raise SystemExit("FAIL: executable cave section raw size is smaller than expected")
            if virtual_size not in ACCEPTED_SECTION_VIRTUAL_SIZES:
                raise SystemExit(
                    f"FAIL: executable cave section virtual size is 0x{virtual_size:x}, "
                    "expected 0x30, 0x80, or 0x100"
                )
            return section_offset + 8

    raise SystemExit("FAIL: could not find expected executable cave section")


def patch_file(path: Path, spec: PatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    cave = build_cave(spec)
    jump = patch_jump(spec)
    label = f"{path} @ 0x{spec.patch_offset:x}"

    section_virtual_size_offset = find_section_virtual_size_offset(data, spec)
    current_site = bytes(data[spec.patch_offset : spec.patch_offset + PATCH_LENGTH])
    current_cave = bytes(data[spec.cave_offset : spec.cave_offset + len(cave)])
    current_virtual_size = struct.unpack_from("<I", data, section_virtual_size_offset)[0]

    if current_site == jump and current_cave == cave and current_virtual_size >= PATCHED_SECTION_VIRTUAL_SIZE:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current_site not in ACCEPTED_PREIMAGES:
        raise SystemExit(
            f"FAIL: {label} has {current_site.hex()}, expected one of "
            f"{', '.join(preimage.hex() for preimage in ACCEPTED_PREIMAGES)} "
            f"or patched {jump.hex()}"
        )

    if any(current_cave):
        raise SystemExit(
            f"FAIL: {path} cave @ 0x{spec.cave_offset:x} is not empty: {current_cave.hex()}"
        )

    if not apply:
        print(
            f"would patch: {label} clamp positive AiDecisionTime "
            f"above {MAX_AI_DECISION_TIME}; current fallback {ACCEPTED_PREIMAGES[current_site]}"
        )
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    data[spec.patch_offset : spec.patch_offset + PATCH_LENGTH] = jump
    data[spec.cave_offset : spec.cave_offset + len(cave)] = cave
    struct.pack_into("<I", data, section_virtual_size_offset, max(current_virtual_size, PATCHED_SECTION_VIRTUAL_SIZE))
    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--apply",
        action="store_true",
        help="write the patch; without this, only validate and print planned changes",
    )
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-ai-decision-clamp-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=list(KNOWN_PATCHES),
        help="DLL paths to patch; defaults to active root and Program copies",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for raw_path in args.paths:
        path = Path(raw_path)
        key = known_key(path)
        patch_file(path, KNOWN_PATCHES[key], args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
