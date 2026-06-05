#!/usr/bin/env python3
"""Patch ManalinkEh.dll AI speculation raw-mana snapshot restore safety.

ai_decision_phase() temporarily replaces the opponent raw_mana_available row
with landsofcolor_controlled[] during AI speculation. The shipped DLLs saved
only that opponent row, but restored both player rows from the stack afterward.
This patch routes the snapshot block through a small executable cave that saves
both rows before preserving the original temporary opponent-row replacement.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCHED_SECTION_VIRTUAL_SIZE = 0x100
PREIMAGE = bytes.fromhex(
    "8b4508"
    "ba01000000"
    "29c2"
    "8d7db4"
    "c1e205"
    "31c0"
    "01d7"
    "8b8c82c0f34e00"
    "890c87"
    "8b8c82a0f44e00"
    "898c82c0f34e00"
    "40"
    "83f808"
    "75e2"
)
PATCH_LENGTH = len(PREIMAGE)
NOP = b"\x90"


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
        patch_offset=0x40DB84,
        patch_vma=0x0240E584,
        after_patch_vma=0x0240E5B6,
        cave_offset=0x495A90,
        cave_vma=0x02497090,
        section_raw=0x495A00,
        section_vma=0x00497000,
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x3D3844,
        patch_vma=0x023D4244,
        after_patch_vma=0x023D4276,
        cave_offset=0x452C90,
        cave_vma=0x02454090,
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

    # Save raw_mana_available[0..1][0..7] into store_raw_mana_available[0..1][0..7].
    cave += b"\x8d\x7d\xb4"  # lea edi, [ebp-0x4c]
    cave += b"\x31\xc0"  # xor eax, eax
    cave += b"\x8b\x0c\x85\xc0\xf3\x4e\x00"  # save_loop: mov ecx, [eax*4+0x4ef3c0]
    cave += b"\x89\x0c\x87"  # mov [edi+eax*4], ecx
    cave += b"\x40"  # inc eax
    cave += b"\x83\xf8\x10"  # cmp eax, 0x10
    cave += b"\x75\xf0"  # jne save_loop

    # Preserve the original temporary opponent-row replacement.
    cave += b"\xba\x01\x00\x00\x00"  # mov edx, 1
    cave += b"\x2b\x55\x08"  # sub edx, [ebp+0x8]
    cave += b"\xc1\xe2\x05"  # shl edx, 0x5
    cave += b"\x31\xc0"  # xor eax, eax
    cave += b"\x8b\x8c\x82\xa0\xf4\x4e\x00"  # replace_loop: mov ecx, [edx+eax*4+0x4ef4a0]
    cave += b"\x89\x8c\x82\xc0\xf3\x4e\x00"  # mov [edx+eax*4+0x4ef3c0], ecx
    cave += b"\x40"  # inc eax
    cave += b"\x83\xf8\x08"  # cmp eax, 0x8
    cave += b"\x75\xec"  # jne replace_loop
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
    raise SystemExit(f"FAIL: no known AI raw-mana snapshot site for {path}")


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
            if virtual_size not in (0x80, PATCHED_SECTION_VIRTUAL_SIZE):
                raise SystemExit(
                    f"FAIL: executable cave section virtual size is 0x{virtual_size:x}, "
                    "expected 0x80 or 0x100"
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

    if (
        current_site == jump
        and current_cave == cave
        and current_virtual_size == PATCHED_SECTION_VIRTUAL_SIZE
    ):
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current_site != PREIMAGE:
        raise SystemExit(
            f"FAIL: {label} has {current_site.hex()}, expected old "
            f"{PREIMAGE.hex()} or patched {jump.hex()}"
        )

    if any(current_cave):
        raise SystemExit(
            f"FAIL: {path} cave @ 0x{spec.cave_offset:x} is not empty: {current_cave.hex()}"
        )

    if not apply:
        print(f"would patch: {label} through cave @ 0x{spec.cave_offset:x}")
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
    struct.pack_into("<I", data, section_virtual_size_offset, PATCHED_SECTION_VIRTUAL_SIZE)
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
        help="preserve a sibling backup before writing, e.g. .before-ai-raw-mana-snapshot-patch",
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
