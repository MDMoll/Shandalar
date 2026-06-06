#!/usr/bin/env python3
"""Patch generic activated damage-prevention checks into ManalinkEh.dll.

The source snapshots already guard generic activated GAA_DAMAGE_PREVENTION*
paths in granted_generic_activated_ability(). This patch adds the matching
runtime guard to the shipped root and Program ManalinkEh.dll copies by routing
the EVENT_CAN_ACTIVATE target-check branch through a small executable cave.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


IMAGE_BASE = 0x02000000
PATCHED_SECTION_VIRTUAL_SIZE = 0x80
ACCEPTED_SECTION_VIRTUAL_SIZES = (0x30, PATCHED_SECTION_VIRTUAL_SIZE, 0x100, 0x200)
PREIMAGE = bytes.fromhex("f6c304746b")
PATCH_LENGTH = len(PREIMAGE)
MASK_GAA_DAMAGE_PREVENTION_FAMILY = 0x0F000000
LAND_CAN_BE_PLAYED_ADDR = 0x004EF190
LCBP_DAMAGE_PREVENTION = 0x04


@dataclass(frozen=True)
class PatchSpec:
    patch_offset: int
    patch_vma: int
    after_patch_vma: int
    return_zero_vma: int
    skip_no_target_vma: int
    cave_offset: int
    cave_vma: int
    section_raw: int
    section_vma: int


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        patch_offset=0x44CB23,
        patch_vma=0x0244D523,
        after_patch_vma=0x0244D528,
        return_zero_vma=0x0244D221,
        skip_no_target_vma=0x0244D593,
        cave_offset=0x495A30,
        cave_vma=0x02497030,
        section_raw=0x495A00,
        section_vma=0x00497000,
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x40F115,
        patch_vma=0x0240FB15,
        after_patch_vma=0x0240FB1A,
        return_zero_vma=0x0240F821,
        skip_no_target_vma=0x0240FB85,
        cave_offset=0x452C30,
        cave_vma=0x02454030,
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
    value = to_vma - from_vma_after_instruction
    return struct.pack("<i", value)


def jmp_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe9" + rel32(from_vma + 5, to_vma)


def je_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\x0f\x84" + rel32(from_vma + 6, to_vma)


def build_cave(spec: PatchSpec) -> bytes:
    cave = bytearray()
    cave += b"\xf7\xc3" + struct.pack("<I", MASK_GAA_DAMAGE_PREVENTION_FAMILY)
    cave += b"\x74\x10"
    cave += b"\xf6\x05" + struct.pack("<I", LAND_CAN_BE_PLAYED_ADDR) + bytes([LCBP_DAMAGE_PREVENTION])
    cave += b"\x75\x07"
    cave += b"\x31\xd2"
    cave += jmp_rel32(spec.cave_vma + len(cave), spec.return_zero_vma)
    cave += b"\xf6\xc3\x04"
    cave += je_rel32(spec.cave_vma + len(cave), spec.skip_no_target_vma)
    cave += jmp_rel32(spec.cave_vma + len(cave), spec.after_patch_vma)
    return bytes(cave)


def patch_jump(spec: PatchSpec) -> bytes:
    return jmp_rel32(spec.patch_vma, spec.cave_vma)


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_PATCHES:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_PATCHES:
            return candidate
    raise SystemExit(f"FAIL: no known generic damage-prevention guard site for {path}")


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
            if raw_size < spec.cave_offset - spec.section_raw + PATCHED_SECTION_VIRTUAL_SIZE:
                raise SystemExit("FAIL: executable cave section raw size is smaller than expected")
            if virtual_size not in ACCEPTED_SECTION_VIRTUAL_SIZES:
                raise SystemExit(
                    f"FAIL: executable cave section virtual size is 0x{virtual_size:x}, "
                    "expected 0x30, 0x80, 0x100, or 0x200"
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
        help="preserve a sibling backup before writing, e.g. .before-generic-damage-prevention-guard",
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
