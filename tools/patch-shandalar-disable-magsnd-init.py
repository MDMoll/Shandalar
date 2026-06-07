#!/usr/bin/env python3
"""Patch Shandalar's MagSnd initializer out of the adventure executable.

Repeated CrossOver hangs around Spell Chain / prompt transitions produced the
same Wine debugger fault pattern: EIP == EDX == 0xfff50de4, outside the loaded
game modules. Earlier patches removed two Shandalar-side calls into the
MagSnd UpdateSnd wrapper, but a later Thoughtseize freeze showed the poisoned
callback path can still be reached after MagSnd.dll is initialized.

This compatibility patch changes Shandalar.exe's MagSnd loader/initializer
wrapper at 0x56cf20 to return the wrapper family's existing "sound unavailable"
status (4) immediately. The DLL is not loaded, later wrapper calls keep their
existing not-initialized short-circuit behavior, and card/duel logic is left
unchanged.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCH_VA = 0x0056CF20
PREIMAGE = bytes.fromhex("558bec83ec08")
PATCHED = bytes.fromhex("b804000000c3")


@dataclass(frozen=True)
class Section:
    name: str
    va: int
    virtual_size: int
    raw_offset: int
    raw_size: int


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_sections(data: bytes) -> tuple[int, list[Section]]:
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise SystemExit("FAIL: not a PE file")

    section_count = struct.unpack_from("<H", data, pe_offset + 6)[0]
    optional_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    optional_header_offset = pe_offset + 24
    image_base = struct.unpack_from("<I", data, optional_header_offset + 28)[0]
    section_offset = optional_header_offset + optional_header_size

    sections: list[Section] = []
    for index in range(section_count):
        offset = section_offset + index * 40
        name = data[offset : offset + 8].split(b"\0", 1)[0].decode("ascii", "replace")
        virtual_size, va, raw_size, raw_offset = struct.unpack_from("<IIII", data, offset + 8)
        sections.append(Section(name, va, virtual_size, raw_offset, raw_size))

    return image_base, sections


def va_to_offset(data: bytes, va: int) -> int:
    image_base, sections = read_sections(data)
    rva = va - image_base
    for section in sections:
        size = max(section.virtual_size, section.raw_size)
        if section.va <= rva < section.va + size:
            raw_delta = rva - section.va
            if raw_delta >= section.raw_size:
                raise SystemExit(f"FAIL: VA 0x{va:x} maps to uninitialized data")
            return section.raw_offset + raw_delta
    raise SystemExit(f"FAIL: VA 0x{va:x} not mapped in PE sections")


def patch_file(path: Path, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    offset = va_to_offset(data, PATCH_VA)
    current = bytes(data[offset : offset + len(PREIMAGE)])
    label = f"{path} MagSnd init wrapper @ VA 0x{PATCH_VA:x}, file 0x{offset:x}"

    if current == PATCHED:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if current != PREIMAGE:
        raise SystemExit(
            f"FAIL: {path} has {current.hex()} at 0x{offset:x}, expected "
            f"old {PREIMAGE.hex()} or patched {PATCHED.hex()}"
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

    data[offset : offset + len(PREIMAGE)] = PATCHED
    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="write the patch")
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-magsnd-init-disable-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=["Shandalar.exe", "Program/Shandalar.exe"],
        help="Shandalar.exe paths to patch; defaults to root and Program copies",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for text_path in args.paths:
        patch_file(Path(text_path), args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
