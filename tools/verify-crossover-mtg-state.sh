#!/usr/bin/env bash
set -euo pipefail

bottle="${1:-MTG}"
bottles_dir="${CROSSOVER_BOTTLES_DIR:-$HOME/Library/Application Support/CrossOver/Bottles}"

export CROSSOVER_BOTTLE_NAME="$bottle"
export CROSSOVER_BOTTLES_DIR="$bottles_dir"

python3 - <<'PY'
from pathlib import Path
import hashlib
import os
import re
import subprocess
import sys


BOTTLE = os.environ["CROSSOVER_BOTTLE_NAME"]
BOTTLES_DIR = Path(os.environ["CROSSOVER_BOTTLES_DIR"])
BASE = BOTTLES_DIR / BOTTLE
INSTALL = BASE / "drive_c" / "Shandalar"
WINE = Path("/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine")

EXPECTED_HASHES = {
    "Shandalar.exe": "ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b",
    "Program/Shandalar.exe": "ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b",
    "Program/zlib.dll": "9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90",
    "Magic.exe": "5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1",
    "Program/Magic.exe": "0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa",
    "FaceMaker.exe": "41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246",
    "Program/FaceMaker.exe": "41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246",
    "ManalinkEh.dll": "6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb",
    "Program/ManalinkEh.dll": "7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517",
}

FACE_SUPPORT = [
    "FaceData.txt",
    "FaceButtons.txt",
    "FaceArt",
    "Program/FaceData.txt",
    "Program/FaceButtons.txt",
    "Program/FaceArt",
]


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    sys.exit(1)


def ok(message: str) -> None:
    print(f"ok: {message}")


def read_text(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def expect_file(path: Path) -> None:
    if not path.exists():
        fail(f"missing {path}")


def expect_hash(rel_path: str, expected: str) -> None:
    path = INSTALL / rel_path
    expect_file(path)
    actual = sha256_file(path)
    if actual != expected:
        fail(f"{rel_path} sha256 {actual} != {expected}")


def section_has_value(text: str, section: str, raw_value: str) -> bool:
    in_section = False
    section_header = f"[{section}]"
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == section_header or stripped.startswith(f"{section_header} "):
            in_section = True
            continue
        if in_section and stripped.startswith("["):
            return False
        if in_section and stripped == raw_value:
            return True
    return False


expect_file(BASE)
expect_file(INSTALL)
ok(f"CrossOver bottle install exists: {INSTALL}")

if WINE.is_file():
    version = subprocess.check_output([str(WINE), "--version"], text=True)
    if "Product Name: CrossOver" not in version or "Product Version: 26.1.0.39808" not in version:
        fail("unexpected CrossOver wine version output")
    ok("CrossOver wine helper reports 26.1.0.39808")
else:
    fail(f"missing CrossOver wine helper: {WINE}")

for rel_path, expected in EXPECTED_HASHES.items():
    expect_hash(rel_path, expected)
ok(f"patched bottle runtime hashes match docs ({len(EXPECTED_HASHES)} checked)")

for rel_path in FACE_SUPPORT:
    expect_file(INSTALL / rel_path)
ok("FaceMaker support files are present in root and Program paths")

for rel_path in ["Shandalar.ini", "Program/Shandalar.ini"]:
    text = read_text(INSTALL / rel_path)
    if not re.search(r"(?m)^Window\s*=\s*2$", text):
        fail(f"{rel_path} does not contain Window = 2")
ok("bottle Shandalar ini files use Window = 2")

user_reg = read_text(BASE / "user.reg")
for exe in ["FaceMaker.exe", "Magic.exe", "Shandalar.exe"]:
    if not section_has_value(user_reg, f"Software\\\\Wine\\\\AppDefaults\\\\{exe}", '"Version"="win7"'):
        fail(f"{exe} app-default Version=win7 not found")
    if not section_has_value(user_reg, f"Software\\\\Wine\\\\AppDefaults\\\\{exe}\\\\Explorer", '"Desktop"="Shandalar1440"'):
        fail(f"{exe} app-default Desktop=Shandalar1440 not found")

if not section_has_value(
    user_reg,
    "Software\\\\Wine\\\\Explorer\\\\Desktops",
    '"Shandalar1440"="1440x1080"',
):
    fail("Shandalar1440=1440x1080 desktop setting not found")
ok("MTG app-default win7 and Shandalar1440 desktop settings are present")

system_reg = read_text(BASE / "system.reg")
for needle in [
    '"ProductName"="Microsoft Windows 7"',
    '"CurrentVersion"="6.1"',
    '"PagingFiles"=str(7):"C:\\\\pagefile.sys 512 1024\\0"',
]:
    if needle not in system_reg:
        fail(f"system.reg missing {needle}")
ok("MTG system registry has Windows 7 identity and paging file setting")

print("CrossOver MTG state checks passed.")
PY
