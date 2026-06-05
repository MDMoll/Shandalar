#!/usr/bin/env python3
"""Patch Magic.exe so missing ShowCoinFlips defaults to off.

Magic stores ShowCoinFlips in the registry under DuelOptions. Existing user
registry values still win; this patch only changes the compiled default used
when that registry value is missing.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
from pathlib import Path


OLD_INSTRUCTION = bytes.fromhex("c7055c72780001000000")
NEW_INSTRUCTION = bytes.fromhex("c7055c72780000000000")

KNOWN_INSTRUCTION_OFFSETS = {
    "Magic.exe": 0x5DB1F,
    "Program/Magic.exe": 0x5DB1F,
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_INSTRUCTION_OFFSETS:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_INSTRUCTION_OFFSETS:
            return candidate
    raise SystemExit(f"FAIL: no known coinflip-default offset for {path}")


def patch_file(path: Path, instruction_offset: int, apply: bool, backup_suffix: str | None) -> None:
    data = path.read_bytes()
    current = data[instruction_offset : instruction_offset + len(OLD_INSTRUCTION)]
    label = f"{path} @ 0x{instruction_offset:x}"

    if current == NEW_INSTRUCTION:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current != OLD_INSTRUCTION:
        raise SystemExit(
            f"FAIL: {label} has {current.hex()}, expected old "
            f"{OLD_INSTRUCTION.hex()} or new {NEW_INSTRUCTION.hex()}"
        )

    if not apply:
        print(f"would patch: {label} ShowCoinFlips default 1 -> 0")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    patched = bytearray(data)
    patched[instruction_offset : instruction_offset + len(NEW_INSTRUCTION)] = NEW_INSTRUCTION
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
        help="preserve a sibling backup before writing, e.g. .before-coinflip-default-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=list(KNOWN_INSTRUCTION_OFFSETS),
        help="Magic.exe paths to patch; defaults to active root and Program copies",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for raw_path in args.paths:
        path = Path(raw_path)
        key = known_key(path)
        patch_file(path, KNOWN_INSTRUCTION_OFFSETS[key], args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
