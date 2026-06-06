#!/usr/bin/env python3
"""Patch AI player-only targets to bypass the generic selector.

The source fix short-circuits select_target_impl() when the AI is resolving a
non-speculating TARGET_ZONE_PLAYERS target. Candidate construction has already
filtered and ordered player targets by preferred_controller, so choosing
candidate zero avoids generic selector side effects without changing human
targeting or mixed creature/player AI decisions.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


HOOK_LENGTH = 17
CAVE_LENGTH = 44


@dataclass(frozen=True)
class PatchSpec:
    hook_offset: int
    hook_vma: int
    cave_offset: int
    cave_vma: int
    return_vma: int
    sub_499050_vma: int
    hook_preimage: bytes


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        hook_offset=0x469583,
        hook_vma=0x02469F83,
        cave_offset=0x495AD0,
        cave_vma=0x024970D0,
        return_vma=0x02469F94,
        sub_499050_vma=0x00499050,
        hook_preimage=bytes.fromhex("898d18fdffffe8c2f002fe8b15e42f7a00"),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        hook_offset=0x429453,
        hook_vma=0x02429E53,
        cave_offset=0x452CD0,
        cave_vma=0x024540D0,
        return_vma=0x02429E64,
        sub_499050_vma=0x00499050,
        hook_preimage=bytes.fromhex("898d18fdffffe8f2f106fe8b15e42f7a00"),
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


def call_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe8" + rel32(from_vma + 5, to_vma)


def build_hook(spec: PatchSpec) -> bytes:
    return jmp_rel32(spec.hook_vma, spec.cave_vma) + b"\x90" * (HOOK_LENGTH - 5)


def build_cave(spec: PatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\x89\x8d\x18\xfd\xff\xff"  # mov [ebp-0x2e8], ecx
    patch += b"\x81\x7d\x1c\x00\x10\x00\x00"  # cmp dword [ebp+0x1c], TARGET_ZONE_PLAYERS
    patch += b"\x75\x0d"  # jne original generic selector path
    patch += b"\x31\xd2"  # xor edx, edx
    patch += b"\x89\x15\xe4\x2f\x7a\x00"  # mov [0x7a2fe4], edx
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.return_vma)

    patch += call_rel32(spec.cave_vma + len(patch), spec.sub_499050_vma)
    patch += b"\x8b\x15\xe4\x2f\x7a\x00"  # mov edx, [0x7a2fe4]
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.return_vma)

    if len(patch) != CAVE_LENGTH:
        raise AssertionError(f"cave length is {len(patch)}, expected {CAVE_LENGTH}")
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
    raise SystemExit(f"FAIL: no known AI player-target patch site for {path}")


def all_zeroes(data: bytes) -> bool:
    return all(byte == 0 for byte in data)


def patch_file(path: Path, spec: PatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    hook = build_hook(spec)
    cave = build_cave(spec)
    label = f"{path} hook @ 0x{spec.hook_offset:x}, cave @ 0x{spec.cave_offset:x}"
    current_hook = bytes(data[spec.hook_offset : spec.hook_offset + HOOK_LENGTH])
    current_cave = bytes(data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH])

    if current_hook == hook and current_cave == cave:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current_hook != spec.hook_preimage:
        raise SystemExit(
            f"FAIL: {path} hook has {current_hook.hex()}, expected old "
            f"{spec.hook_preimage.hex()} or patched {hook.hex()}"
        )

    if not all_zeroes(current_cave):
        raise SystemExit(
            f"FAIL: {path} cave has {current_cave.hex()}, expected zero-filled cave "
            f"or patched {cave.hex()}"
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

    data[spec.hook_offset : spec.hook_offset + HOOK_LENGTH] = hook
    data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH] = cave
    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="write the patch")
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-ai-player-target-selection-patch",
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
