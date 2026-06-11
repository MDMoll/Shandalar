#!/usr/bin/env python3
"""Patch Shandalar.dll generic-cost autotap ordering.

Shandalar's adventure-duel DLL normally tries human autotap in two land passes:
first "relevant basics only", then "all lands".  For a pure generic cost of
two or more, that first pass can consume a basic land before the all-land pass
gets a chance to use a two-mana nonbasic such as Jungle Basin, leaving excess
mana floating.

This patch routes the first-pass flag write through the existing executable
.cdxai cave section.  The cave keeps the original basic-only flags for normal
costs, but uses the all-land flags for pure generic costs of 2+.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
from pathlib import Path


HOOK_OFFSET = 0x825C0
CAVE_OFFSET = 0x1174980

HOOK_ORIGINAL = bytes.fromhex("c704241e000000")
HOOK_PATCHED = bytes.fromhex("e8bb9fcd009090")

CAVE_BYTES = bytes.fromhex(
    # Preserve eax.  A call into this cave pushes the return address, and this
    # push moves the original outgoing [esp] argument slot to [esp+8].
    "50"
    # eax = pay_mana[COLOR_COLORLESS]
    "a190268e00"
    # If high flag bits are set, this is not a simple pure-generic cost.
    "a90000ffff"
    "753b"
    # Need at least two generic mana before preferring a 2-mana nonbasic.
    "6683f802"
    "7c35"
    # No colored/artifact/hybrid/X pay requirements may be pending.
    "a194268e00"
    "0b0598268e00"
    "0b059c268e00"
    "0b05a0268e00"
    "0b05a4268e00"
    "0b05a8268e00"
    "0b05ac268e00"
    "750a"
    # Pure generic 2+: first pass may use nonbasic lands too.
    "c74424081c000000"
    "58"
    "c3"
    # Default: preserve original basic-only first pass.
    "c74424081e000000"
    "58"
    "c3"
)

BUGGY_CAVE_BYTES = bytes.fromhex(
    # Initial local draft wrote [esp+4] after saving eax, which targets the
    # call return address.  Accept it only to repair to CAVE_BYTES.
    "50"
    "a190268e00"
    "a90000ffff"
    "753b"
    "6683f802"
    "7c35"
    "a194268e00"
    "0b0598268e00"
    "0b059c268e00"
    "0b05a0268e00"
    "0b05a4268e00"
    "0b05a8268e00"
    "0b05ac268e00"
    "750a"
    "c74424041c000000"
    "58"
    "c3"
    "c74424041e000000"
    "58"
    "c3"
)

EXPECTED_CAVE_BLANK = bytes(len(CAVE_BYTES))


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def backup_path(path: Path) -> Path:
    return path.with_name(path.stem + ".before-generic-autotap-order-patch" + path.suffix)


def patch_one(path: Path, dry_run: bool) -> str:
    data = bytearray(path.read_bytes())

    hook = bytes(data[HOOK_OFFSET : HOOK_OFFSET + len(HOOK_PATCHED)])
    cave = bytes(data[CAVE_OFFSET : CAVE_OFFSET + len(CAVE_BYTES)])

    if hook == HOOK_PATCHED:
        if cave == CAVE_BYTES:
            return f"ok: {path} generic autotap hook already patched; sha256 {sha256(data)}"
        if cave == BUGGY_CAVE_BYTES:
            data[CAVE_OFFSET : CAVE_OFFSET + len(CAVE_BYTES)] = CAVE_BYTES
            if dry_run:
                return f"ok: would repair {path} generic autotap cave; new sha256 {sha256(data)}"

            backup = backup_path(path)
            if not backup.exists():
                shutil.copy2(path, backup)

            path.write_bytes(data)
            return f"ok: repaired {path} generic autotap cave; backup {backup.name}; sha256 {sha256(data)}"

        raise SystemExit(
            f"FAIL: {path} cave @ 0x{CAVE_OFFSET:x} is not blank, not repaired, "
            f"and not this patch"
        )

    if hook != HOOK_ORIGINAL:
        raise SystemExit(
            f"FAIL: {path} hook @ 0x{HOOK_OFFSET:x} is {hook.hex()}, "
            f"expected original {HOOK_ORIGINAL.hex()} or patched {HOOK_PATCHED.hex()}"
        )

    if cave != EXPECTED_CAVE_BLANK:
        raise SystemExit(
            f"FAIL: {path} cave @ 0x{CAVE_OFFSET:x} is not blank and not this patch"
        )

    data[HOOK_OFFSET : HOOK_OFFSET + len(HOOK_PATCHED)] = HOOK_PATCHED
    data[CAVE_OFFSET : CAVE_OFFSET + len(CAVE_BYTES)] = CAVE_BYTES

    if dry_run:
        return f"ok: would patch {path}; new sha256 {sha256(data)}"

    backup = backup_path(path)
    if not backup.exists():
        shutil.copy2(path, backup)

    path.write_bytes(data)
    return f"ok: patched {path}; backup {backup.name}; sha256 {sha256(data)}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "paths",
        nargs="*",
        default=["Shandalar.dll", "Program/Shandalar.dll"],
        help="Shandalar.dll paths to patch",
    )
    parser.add_argument("--dry-run", action="store_true", help="validate only")
    args = parser.parse_args()

    for raw in args.paths:
        print(patch_one(Path(raw), args.dry_run))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
