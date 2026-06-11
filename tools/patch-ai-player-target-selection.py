#!/usr/bin/env python3
"""Patch fragile AI target-selection paths to bypass the generic selector.

The source fix short-circuits select_target_impl() when the AI is resolving a
non-speculating TARGET_ZONE_PLAYERS target. Candidate construction has already
filtered and ordered player targets by preferred_controller, so choosing
candidate zero avoids generic selector side effects without changing human
targeting or mixed creature/player AI decisions.

Shandalar adventure duels load Shandalar.dll, whose C++ Target::real_select_target()
has the same failure class but a different implementation. The guarded
Shandalar hook skips its old activation/target presentation selector for pure
player targets and pure in-play permanent targets, and only after legal
candidates have already been built.
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
SHANDALAR_HOOK_LENGTH = 18
SHANDALAR_CAVE_LENGTH = 64
OLD_SHANDALAR_CAVE_LENGTH = 44


@dataclass(frozen=True)
class PatchSpec:
    hook_offset: int
    hook_vma: int
    cave_offset: int
    cave_vma: int
    return_vma: int
    sub_499050_vma: int
    hook_preimage: bytes


@dataclass(frozen=True)
class ShandalarPatchSpec:
    hook_offset: int
    hook_vma: int
    cave_offset: int
    cave_vma: int
    choose_vma: int
    original_continue_vma: int
    selector_vma: int
    chosen_index_addr: int
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

KNOWN_SHANDALAR_PATCHES = {
    "Shandalar.dll": ShandalarPatchSpec(
        hook_offset=0xCB16,
        hook_vma=0x0600D716,
        cave_offset=0x1174920,
        cave_vma=0x06D5D120,
        choose_vma=0x0600D73F,
        original_continue_vma=0x0600D728,
        selector_vma=0x004C42F8,
        chosen_index_addr=0x0094D46C,
        hook_preimage=bytes.fromhex("898d3cfdffffb8f8424c00ffd0a16cd49400"),
    ),
    "Program/Shandalar.dll": ShandalarPatchSpec(
        hook_offset=0xCB16,
        hook_vma=0x0600D716,
        cave_offset=0x1174920,
        cave_vma=0x06D5D120,
        choose_vma=0x0600D73F,
        original_continue_vma=0x0600D728,
        selector_vma=0x004C42F8,
        chosen_index_addr=0x0094D46C,
        hook_preimage=bytes.fromhex("898d3cfdffffb8f8424c00ffd0a16cd49400"),
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


def mov_abs_eax(addr: int) -> bytes:
    return b"\xa3" + struct.pack("<I", addr)


def mov_eax_abs(addr: int) -> bytes:
    return b"\xa1" + struct.pack("<I", addr)


def build_shandalar_hook(spec: ShandalarPatchSpec) -> bytes:
    return jmp_rel32(spec.hook_vma, spec.cave_vma) + b"\x90" * (SHANDALAR_HOOK_LENGTH - 5)


def build_shandalar_player_only_cave(spec: ShandalarPatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\x89\x8d\x3c\xfd\xff\xff"  # mov [ebp-0x2c4], ecx
    patch += b"\x83\x7f\x08\x01"  # cmp dword [edi+0x8], AI
    patch += b"\x75\x15"  # jne original selector path
    patch += b"\x81\x7f\x14\x00\x10\x00\x00"  # cmp dword [edi+0x14], TARGET_ZONE_PLAYERS
    patch += b"\x75\x0c"  # jne original selector path
    patch += b"\x31\xc0"  # xor eax, eax
    patch += mov_abs_eax(spec.chosen_index_addr)  # mov [chosen_index], eax
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.choose_vma)

    patch += b"\xb8" + struct.pack("<I", spec.selector_vma)  # mov eax, old selector
    patch += b"\xff\xd0"  # call eax
    patch += mov_eax_abs(spec.chosen_index_addr)  # mov eax, [chosen_index]
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.original_continue_vma)

    if len(patch) > SHANDALAR_CAVE_LENGTH:
        raise AssertionError(f"Shandalar cave length is {len(patch)}, exceeds {SHANDALAR_CAVE_LENGTH}")
    patch += bytes(SHANDALAR_CAVE_LENGTH - len(patch))
    return bytes(patch)


def build_shandalar_cave(spec: ShandalarPatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\x89\x8d\x3c\xfd\xff\xff"  # mov [ebp-0x2c4], ecx
    patch += b"\x83\x7f\x08\x01"  # cmp dword [edi+0x8], AI
    patch += b"\x75\x1e"  # jne original selector path
    patch += b"\x81\x7f\x14\x00\x10\x00\x00"  # cmp dword [edi+0x14], TARGET_ZONE_PLAYERS
    patch += b"\x74\x09"  # je choose candidate zero
    patch += b"\x81\x7f\x14\x00\x02\x00\x00"  # cmp dword [edi+0x14], TARGET_ZONE_IN_PLAY
    patch += b"\x75\x0c"  # jne original selector path
    patch += b"\x31\xc0"  # xor eax, eax
    patch += mov_abs_eax(spec.chosen_index_addr)  # mov [chosen_index], eax
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.choose_vma)

    patch += b"\xb8" + struct.pack("<I", spec.selector_vma)  # mov eax, old selector
    patch += b"\xff\xd0"  # call eax
    patch += mov_eax_abs(spec.chosen_index_addr)  # mov eax, [chosen_index]
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.original_continue_vma)

    if len(patch) > SHANDALAR_CAVE_LENGTH:
        raise AssertionError(f"Shandalar cave length is {len(patch)}, exceeds {SHANDALAR_CAVE_LENGTH}")
    patch += bytes(SHANDALAR_CAVE_LENGTH - len(patch))
    return bytes(patch)


def build_old_shandalar_cave(spec: ShandalarPatchSpec) -> bytes:
    patch = bytearray()

    patch += b"\x89\x8d\x3c\xfd\xff\xff"  # mov [ebp-0x2c4], ecx
    patch += b"\x81\x7f\x14\x00\x10\x00\x00"  # cmp dword [edi+0x14], TARGET_ZONE_PLAYERS
    patch += b"\x75\x0c"  # jne original selector path
    patch += b"\x31\xc0"  # xor eax, eax
    patch += mov_abs_eax(spec.chosen_index_addr)  # mov [chosen_index], eax
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.choose_vma)

    patch += b"\xb8" + struct.pack("<I", spec.selector_vma)  # mov eax, old selector
    patch += b"\xff\xd0"  # call eax
    patch += mov_eax_abs(spec.chosen_index_addr)  # mov eax, [chosen_index]
    patch += jmp_rel32(spec.cave_vma + len(patch), spec.original_continue_vma)

    if len(patch) != OLD_SHANDALAR_CAVE_LENGTH:
        raise AssertionError(f"old Shandalar cave length is {len(patch)}, expected {OLD_SHANDALAR_CAVE_LENGTH}")
    return bytes(patch)


def build_misaligned_ai_guard_shandalar_cave(spec: ShandalarPatchSpec) -> bytes:
    """Previous AI-guard build with the first jne landing two bytes early."""
    cave = bytearray(build_shandalar_player_only_cave(spec))
    cave[11] = 0x13
    return bytes(cave)


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_PATCHES:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_PATCHES:
            return candidate
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_SHANDALAR_PATCHES:
            return candidate
    raise SystemExit(f"FAIL: no known AI player-target patch site for {path}")


def known_spec(path: Path) -> PatchSpec | ShandalarPatchSpec:
    key = known_key(path)
    if key in KNOWN_PATCHES:
        return KNOWN_PATCHES[key]
    return KNOWN_SHANDALAR_PATCHES[key]


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

    if current_hook not in (spec.hook_preimage, hook):
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


def patch_shandalar_file(path: Path, spec: ShandalarPatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    hook = build_shandalar_hook(spec)
    cave = build_shandalar_cave(spec)
    player_only_cave = build_shandalar_player_only_cave(spec)
    old_cave = build_old_shandalar_cave(spec) + bytes(SHANDALAR_CAVE_LENGTH - OLD_SHANDALAR_CAVE_LENGTH)
    misaligned_cave = build_misaligned_ai_guard_shandalar_cave(spec)
    label = f"{path} hook @ 0x{spec.hook_offset:x}, cave @ 0x{spec.cave_offset:x}"
    current_hook = bytes(data[spec.hook_offset : spec.hook_offset + SHANDALAR_HOOK_LENGTH])
    current_cave = bytes(data[spec.cave_offset : spec.cave_offset + SHANDALAR_CAVE_LENGTH])

    if current_hook == hook and current_cave == cave:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current_hook not in (spec.hook_preimage, hook):
        raise SystemExit(
            f"FAIL: {path} hook has {current_hook.hex()}, expected old "
            f"{spec.hook_preimage.hex()} or patched {hook.hex()}"
        )

    if current_cave not in (old_cave, misaligned_cave, player_only_cave) and not all_zeroes(current_cave):
        raise SystemExit(
            f"FAIL: {path} cave has {current_cave.hex()}, expected zero-filled cave "
            f"old patched cave {old_cave.hex()}, player-only patched cave "
            f"{player_only_cave.hex()}, misaligned AI-guard cave {misaligned_cave.hex()}, "
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

    data[spec.hook_offset : spec.hook_offset + SHANDALAR_HOOK_LENGTH] = hook
    data[spec.cave_offset : spec.cave_offset + SHANDALAR_CAVE_LENGTH] = cave
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
        default=["ManalinkEh.dll", "Program/ManalinkEh.dll", "Shandalar.dll", "Program/Shandalar.dll"],
        help="DLL paths to patch; defaults to root and Program ManalinkEh.dll and Shandalar.dll",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for text_path in args.paths:
        path = Path(text_path)
        spec = known_spec(path)
        if isinstance(spec, ShandalarPatchSpec):
            patch_shandalar_file(path, spec, args.apply, args.backup_suffix)
        else:
            patch_file(path, spec, args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
