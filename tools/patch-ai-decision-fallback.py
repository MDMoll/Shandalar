#!/usr/bin/env python3
"""Patch ManalinkEh.dll AI decision-time fallback to 270.

The active duel engine normally reads AiDecisionTime from config.txt. This
patch hardens the compiled fallback used when that setting is missing or
invalid, without changing the configured value for normal installs.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
from pathlib import Path


ACCEPTED_OLD = {
    bytes.fromhex("1d150000"): 5405,
    bytes.fromhex("1c020000"): 540,
}
NEW = bytes.fromhex("0e010000")  # 270
OPCODE_MOV_EBX_IMM32 = b"\xbb"

KNOWN_INSTRUCTION_OFFSETS = {
    "ManalinkEh.dll": 0x40D0E7,
    "Program/ManalinkEh.dll": 0x3D2DA7,
}

SUPERSEDING_CLAMP_SITES = {
    "ManalinkEh.dll": (0x40D0E1, bytes.fromhex("e97a950800909090909090")),
    "Program/ManalinkEh.dll": (0x3D2DA1, bytes.fromhex("e9ba080800909090909090")),
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def patch_file(
    path: Path, key: str, instruction_offset: int, apply: bool, backup_suffix: str | None
) -> None:
    data = path.read_bytes()
    if key in SUPERSEDING_CLAMP_SITES:
        clamp_offset, clamp_bytes = SUPERSEDING_CLAMP_SITES[key]
        current_clamp = data[clamp_offset : clamp_offset + len(clamp_bytes)]
        if current_clamp == clamp_bytes:
            print(f"ok: {path} has superseding AI decision clamp patch; sha256 {sha256_file(path)}")
            return

    opcode = data[instruction_offset : instruction_offset + len(OPCODE_MOV_EBX_IMM32)]
    if opcode != OPCODE_MOV_EBX_IMM32:
        raise SystemExit(
            f"FAIL: {path} @ 0x{instruction_offset:x} has opcode {opcode.hex()}, "
            f"expected {OPCODE_MOV_EBX_IMM32.hex()}"
        )

    immediate_offset = instruction_offset + len(OPCODE_MOV_EBX_IMM32)
    current = data[immediate_offset : immediate_offset + len(NEW)]
    label = f"{path} @ 0x{instruction_offset:x}"

    if current == NEW:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current not in ACCEPTED_OLD:
        raise SystemExit(
            f"FAIL: {label} has {current.hex()}, expected one of "
            f"{', '.join(old.hex() for old in ACCEPTED_OLD)} or new {NEW.hex()}"
        )

    if not apply:
        print(f"would patch: {label} {ACCEPTED_OLD[current]} -> 270")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    patched = bytearray(data)
    patched[immediate_offset : immediate_offset + len(NEW)] = NEW
    path.write_bytes(patched)
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
        help="preserve a sibling backup before writing, e.g. .before-ai-decision-fallback-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=list(KNOWN_INSTRUCTION_OFFSETS),
        help="DLL paths to patch; defaults to the active root and Program DLLs",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for raw_path in args.paths:
        path = Path(raw_path)
        key = path.as_posix()
        if key not in KNOWN_INSTRUCTION_OFFSETS:
            # Absolute copied-install paths keep the same final relative shape.
            parts = path.parts
            rel_key = None
            for index in range(len(parts)):
                candidate = Path(*parts[index:]).as_posix()
                if candidate in KNOWN_INSTRUCTION_OFFSETS:
                    rel_key = candidate
                    break
            if rel_key is None:
                raise SystemExit(f"FAIL: no known AI fallback offset for {path}")
            key = rel_key
        patch_file(path, key, KNOWN_INSTRUCTION_OFFSETS[key], args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
