#!/usr/bin/env python3
"""Patch Myr Moonvessel to resolve its dies mana ability as a real trigger.

The shipped card code called graveyard_from_play() and produce_mana() directly
during EVENT_GRAVEYARD_FROM_PLAY.  That can put mana/UI work inside the
graveyard transition itself.  The source fix uses this_dies_trigger(), matching
Cathodion's "dies, add colorless mana" pattern.  This helper applies the same
small function rewrite to the active root and Program ManalinkEh.dll layouts
after validating exact preimage bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCH_LENGTH = 0x50
PRODUCE_MANA_VMA = 0x0049D490
RESOLVE_TRIGGER_MANDATORY = 2


@dataclass(frozen=True)
class PatchSpec:
    patch_offset: int
    patch_vma: int
    this_dies_trigger_vma: int
    preimage: bytes


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        patch_offset=0x0B9D40,
        patch_vma=0x020BA740,
        this_dies_trigger_vma=0x024485B0,
        preimage=bytes.fromhex(
            "5589e55383ec0c8b45108b5d08894424088b450c89442404891c24"
            "e80068370085c07418c744240801000000c744240400000000891c24"
            "e8142d3efe83c40c31c05b5dc38db6000000008dbf00000000"
        ),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x0B1CD0,
        patch_vma=0x020B26D0,
        this_dies_trigger_vma=0x0240A5F0,
        preimage=bytes.fromhex(
            "5589e55383ec0c8b45108b5d08894424088b450c89442404891c24"
            "e8301b340085c07418c744240801000000c744240400000000891c24"
            "e884ad3efe83c40c31c05b5dc38db6000000008dbf00000000"
        ),
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


def call_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe8" + rel32(from_vma + 5, to_vma)


def build_patch(spec: PatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\x55"  # push ebp
    patch += b"\x89\xe5"  # mov ebp, esp
    patch += b"\x53"  # push ebx
    patch += b"\x83\xec\x10"  # sub esp, 0x10
    patch += b"\x8b\x45\x10"  # mov eax, [ebp+0x10]
    patch += b"\x8b\x5d\x08"  # mov ebx, [ebp+0x8]
    patch += b"\x89\x44\x24\x08"  # mov [esp+0x8], eax
    patch += b"\x8b\x45\x0c"  # mov eax, [ebp+0xc]
    patch += b"\xc7\x44\x24\x0c" + struct.pack("<I", RESOLVE_TRIGGER_MANDATORY)
    patch += b"\x89\x44\x24\x04"  # mov [esp+0x4], eax
    patch += b"\x89\x1c\x24"  # mov [esp], ebx
    patch += call_rel32(spec.patch_vma + len(patch), spec.this_dies_trigger_vma)

    patch += b"\x85\xc0"  # test eax, eax
    patch += b"\x74\x18"  # je cleanup
    patch += b"\xc7\x44\x24\x08\x01\x00\x00\x00"  # mov [esp+0x8], 1
    patch += b"\xc7\x44\x24\x04\x00\x00\x00\x00"  # mov [esp+0x4], COLOR_COLORLESS
    patch += b"\x89\x1c\x24"  # mov [esp], ebx
    patch += call_rel32(spec.patch_vma + len(patch), PRODUCE_MANA_VMA)

    patch += b"\x83\xc4\x10"  # add esp, 0x10
    patch += b"\x31\xc0"  # xor eax, eax
    patch += b"\x5b"  # pop ebx
    patch += b"\x5d"  # pop ebp
    patch += b"\xc3"  # ret
    patch += b"\x8d\x74\x26\x00"  # nop padding

    if len(patch) != PATCH_LENGTH:
        raise AssertionError(f"patch length is {len(patch)}, expected {PATCH_LENGTH}")
    return bytes(patch)


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_PATCHES:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_PATCHES:
            return candidate
    raise SystemExit(f"FAIL: no known Myr Moonvessel patch site for {path}")


def patch_file(path: Path, spec: PatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    patch = build_patch(spec)
    label = f"{path} @ 0x{spec.patch_offset:x}"
    current = bytes(data[spec.patch_offset : spec.patch_offset + PATCH_LENGTH])

    if current == patch:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current != spec.preimage:
        raise SystemExit(
            f"FAIL: {label} has {current.hex()}, expected old "
            f"{spec.preimage.hex()} or patched {patch.hex()}"
        )

    if not apply:
        print(f"would patch: {label}")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    data[spec.patch_offset : spec.patch_offset + PATCH_LENGTH] = patch
    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="write the patch")
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-myr-moonvessel-dies-trigger-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=["ManalinkEh.dll", "Program/ManalinkEh.dll"],
        help="DLL paths to patch; defaults to root and Program ManalinkEh.dll",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for text_path in args.paths:
        path = Path(text_path)
        key = known_key(path)
        patch_file(path, KNOWN_PATCHES[key], args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
