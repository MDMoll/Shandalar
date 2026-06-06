#!/usr/bin/env python3
"""Patch Bojuka Bog to avoid generic AI target-selection stalls.

The source fix uses pick_player_duh(player, card, 1-player, 0): AI and Duh
mode directly target the opponent, while ordinary human play can still choose
any legal player. This helper applies the equivalent in-place patch to the
active root and Program ManalinkEh.dll layouts after validating exact preimage
bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCH_LENGTH = 117


@dataclass(frozen=True)
class PatchSpec:
    patch_offset: int
    patch_vma: int
    mana_vma: int
    pick_player_duh_vma: int
    get_card_instance_vma: int
    rfg_whole_graveyard_vma: int
    preimage: bytes


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        patch_offset=0x3F63E0,
        patch_vma=0x023F6DE0,
        mana_vma=0x023F6DC6,
        pick_player_duh_vma=0x0246BF40,
        get_card_instance_vma=0x0241DE70,
        rfg_whole_graveyard_vma=0x024205E0,
        preimage=bytes.fromhex(
            "8d45a4c744240c000000008944240889742404891c24e8d53f0700"
            "c745b000100000c745e80000000089742404891c24e85b7002008d55a4"
            "8945a0c6403600891424e87941070085c0749b8d45a4c7442404bc634c02"
            "890424e8a250070085c074848b55a08b4274890424e890970200e971ffffff"
        ),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        patch_offset=0x3BC630,
        patch_vma=0x023BD030,
        mana_vma=0x023BD016,
        pick_player_duh_vma=0x0242BE10,
        get_card_instance_vma=0x023E2970,
        rfg_whole_graveyard_vma=0x023E4D50,
        preimage=bytes.fromhex(
            "8d45a4c744240c000000008944240889742404891c24e855dc0600"
            "c745b000100000c745e80000000089742404891c24e80b5902008d55a4"
            "8945a0c6403600891424e8f9dd060085c0749b8d45a4c744240436104802"
            "890424e822ed060085c074848b55a08b4274890424e8b07c0200e971ffffff"
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

    patch += b"\x8b\x40\x74"  # mov eax, [eax+0x74]
    patch += b"\x89\x04\x24"  # mov [esp], eax
    patch += call_rel32(spec.patch_vma + len(patch), spec.rfg_whole_graveyard_vma)
    patch += jmp_rel32(spec.patch_vma + len(patch), spec.mana_vma)

    if len(patch) > PATCH_LENGTH:
        raise AssertionError(f"patch length is {len(patch)}, exceeds {PATCH_LENGTH}")
    patch += b"\x90" * (PATCH_LENGTH - len(patch))
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
    raise SystemExit(f"FAIL: no known Bojuka Bog patch site for {path}")


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
        help="preserve a sibling backup before writing, e.g. .before-bojuka-bog-target-patch",
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
