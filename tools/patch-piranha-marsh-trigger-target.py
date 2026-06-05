#!/usr/bin/env python3
"""Patch Piranha Marsh to avoid generic AI target-prompt stalls.

The source fix uses pick_player_duh(player, card, 1-player, 0): AI and Duh
mode directly target the opponent, while ordinary human play can still choose
any legal player. This helper applies the equivalent small in-place patch to
the active root and Program ManalinkEh.dll layouts after validating exact
preimage bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCH_LENGTH = 75


@dataclass(frozen=True)
class PatchSpec:
    patch_offset: int
    patch_vma: int
    mana_vma: int
    pick_player_duh_vma: int
    get_card_instance_vma: int
    lose_life_vma: int
    preimage: bytes


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        patch_offset=0x3FE7A0,
        patch_vma=0x023FF1A0,
        mana_vma=0x023FF186,
        pick_player_duh_vma=0x0246BF40,
        get_card_instance_vma=0x0241DE70,
        lose_life_vma=0x0243DC70,
        preimage=bytes.fromhex(
            "8d45a4c744240c000000008944240889742404891c24e815bc0600"
            "c745e800000000c745b00010000089742404891c24e89bec01008d55a4"
            "8945a0c6403600891424e8b9bd060085c0749b"
        ),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x3C4930,
        patch_vma=0x023C5330,
        mana_vma=0x023C5316,
        pick_player_duh_vma=0x0242BE10,
        get_card_instance_vma=0x023E2970,
        lose_life_vma=0x02400280,
        preimage=bytes.fromhex(
            "8d45a4c744240c000000008944240889742404891c24e855590600"
            "c745e800000000c745b00010000089742404891c24e80bd601008d55a4"
            "8945a0c6403600891424e8f95a060085c0749b"
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


def jmp_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe9" + rel32(from_vma + 5, to_vma)


def build_patch(spec: PatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\xc7\x44\x24\x0c\x00\x00\x00\x00"  # mov dword [esp+0xc], 0
    patch += b"\xb8\x01\x00\x00\x00"  # mov eax, 1
    patch += b"\x29\xd8"  # sub eax, ebx
    patch += b"\x89\x44\x24\x08"  # mov [esp+0x8], eax
    patch += b"\x89\x74\x24\x04"  # mov [esp+0x4], esi
    patch += b"\x89\x1c\x24"  # mov [esp], ebx
    patch += call_rel32(spec.patch_vma + len(patch), spec.pick_player_duh_vma)

    patch += b"\x85\xc0"  # test eax, eax
    patch += b"\x0f\x84" + rel32(spec.patch_vma + len(patch) + 6, spec.mana_vma)

    patch += b"\x89\x74\x24\x04"  # mov [esp+0x4], esi
    patch += b"\x89\x1c\x24"  # mov [esp], ebx
    patch += call_rel32(spec.patch_vma + len(patch), spec.get_card_instance_vma)

    patch += b"\xc7\x44\x24\x04\x01\x00\x00\x00"  # mov dword [esp+0x4], 1
    patch += b"\x8b\x40\x74"  # mov eax, [eax+0x74]
    patch += b"\x89\x04\x24"  # mov [esp], eax
    patch += call_rel32(spec.patch_vma + len(patch), spec.lose_life_vma)
    patch += jmp_rel32(spec.patch_vma + len(patch), spec.mana_vma)

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
    raise SystemExit(f"FAIL: no known Piranha Marsh patch site for {path}")


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
        help="preserve a sibling backup before writing, e.g. .before-piranha-marsh-target-patch",
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
        spec = KNOWN_PATCHES[known_key(path)]
        patch_file(path, spec, args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
