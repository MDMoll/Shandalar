#!/usr/bin/env python3
"""Patch Shandalar's WinMM timer callback down to the tick increment only.

Adventure-duel freezes in CrossOver repeatedly produced the same Wine debugger
fault pattern: EIP == EDX == 0xfff50de4, outside the loaded game modules. The
fault persisted after disabling MagSnd and Statwin's MagVid loader, and a
later pre-card duel-start freeze showed that neither MagSnd.dll nor magvid.dll
was loaded.

Shandalar.exe still owns a WinMM periodic timer callback at 0x4ce8cd. The
callback's required job is to increment the game tick counter at 0x589df0.
Its first-run body also touches thread handles and priority from Wine's timer
worker thread. This compatibility patch preserves the tick counter while
removing that thread bookkeeping:

    inc dword ptr [0x589df0]
    ret 0x14

The 0x14 stack pop matches the five-argument CALLBACK convention used by
timeSetEvent. This keeps Shandalar's timer wait helpers from hanging while
minimizing callback-thread interaction with Wine.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


PATCH_VA = 0x004CE8CD
PATCH_SIZE = 0x50
PREIMAGE_PREFIX = bytes.fromhex("558bec535657833d947d5b00000f853d000000")
PATCH_PREFIX = bytes.fromhex("ff05f09d5800c21400")
PATCHED = PATCH_PREFIX + bytes([0x90]) * (PATCH_SIZE - len(PATCH_PREFIX))


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
    current = bytes(data[offset : offset + PATCH_SIZE])
    label = f"{path} WinMM timer callback @ VA 0x{PATCH_VA:x}, file 0x{offset:x}"

    if current == PATCHED:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if not current.startswith(PREIMAGE_PREFIX):
        raise SystemExit(
            f"FAIL: {path} has prefix {current[:len(PREIMAGE_PREFIX)].hex()} "
            f"at 0x{offset:x}, expected old {PREIMAGE_PREFIX.hex()} or patched "
            f"{PATCHED[:len(PREIMAGE_PREFIX)].hex()}"
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

    data[offset : offset + PATCH_SIZE] = PATCHED
    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="write the patch")
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-minimal-winmm-timer-callback-patch",
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
