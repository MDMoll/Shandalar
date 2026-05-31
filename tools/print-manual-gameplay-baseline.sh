#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bottle="${1:-MTG}"
bottles_dir="${CROSSOVER_BOTTLES_DIR:-$HOME/Library/Application Support/CrossOver/Bottles}"

export SHANDALAR_REPO_ROOT="$repo_root"
export SHANDALAR_CROSSOVER_BOTTLE="$bottle"
export SHANDALAR_CROSSOVER_BOTTLES_DIR="$bottles_dir"

python3 - <<'PY'
from pathlib import Path
import hashlib
import os
import platform
import re
import subprocess
from datetime import datetime


REPO = Path(os.environ["SHANDALAR_REPO_ROOT"])
BOTTLE = os.environ["SHANDALAR_CROSSOVER_BOTTLE"]
BOTTLES_DIR = Path(os.environ["SHANDALAR_CROSSOVER_BOTTLES_DIR"])
BOTTLE_ROOT = BOTTLES_DIR / BOTTLE
INSTALL = BOTTLE_ROOT / "drive_c" / "Shandalar"
WINE = Path("/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine")

HASH_TARGETS = [
    ("Repo Shandalar.exe", REPO / "Shandalar.exe"),
    ("Repo Magic.exe", REPO / "Magic.exe"),
    ("Repo Program/Magic.exe", REPO / "Program" / "Magic.exe"),
    ("Repo ManalinkEh.dll", REPO / "ManalinkEh.dll"),
    ("Repo Program/ManalinkEh.dll", REPO / "Program" / "ManalinkEh.dll"),
    ("Bottle Shandalar.exe", INSTALL / "Shandalar.exe"),
    ("Bottle Magic.exe", INSTALL / "Magic.exe"),
    ("Bottle Program/Magic.exe", INSTALL / "Program" / "Magic.exe"),
    ("Bottle ManalinkEh.dll", INSTALL / "ManalinkEh.dll"),
    ("Bottle Program/ManalinkEh.dll", INSTALL / "Program" / "ManalinkEh.dll"),
]


def run_text(args, cwd=REPO):
    try:
        result = subprocess.run(
            args,
            cwd=cwd,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
    except FileNotFoundError:
        return "not available"
    text = result.stdout.strip()
    return text or f"exit {result.returncode}, no output"


def file_sha(path):
    if not path.is_file():
        return "missing"
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_text(path):
    if not path.is_file():
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def section_value(text, section, key):
    in_section = False
    header = f"[{section}]"
    pattern = re.compile(rf'^"{re.escape(key)}"="?(.*?)"?$')
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == header or stripped.startswith(f"{header} "):
            in_section = True
            continue
        if in_section and stripped.startswith("["):
            return "not found"
        if in_section:
            match = pattern.match(stripped)
            if match:
                return match.group(1)
    return "not found"


def system_windows_version(system_reg):
    for section in [
        "Software\\\\Wow6432Node\\\\Microsoft\\\\Windows NT\\\\CurrentVersion",
        "Software\\\\Microsoft\\\\Windows NT\\\\CurrentVersion",
    ]:
        product = section_value(system_reg, section, "ProductName")
        version = section_value(system_reg, section, "CurrentVersion")
        if product != "not found" and version != "not found":
            return f"{product} ({version})"
        if product != "not found":
            return product
    return "not found"


def wine_version():
    if not WINE.is_file():
        return "missing CrossOver wine helper"
    return run_text([str(WINE), "--version"], cwd=REPO).replace("\n", "; ")


def markdown_row(left, right):
    return f"| {left} | {right} |"


branch = run_text(["git", "branch", "--show-current"])
commit = run_text(["git", "rev-parse", "--short", "HEAD"])
status = run_text(["git", "status", "--short", "--untracked-files=all"])
status = "clean" if status == "exit 0, no output" else status.replace("\n", "<br>")

user_reg = read_text(BOTTLE_ROOT / "user.reg")
system_reg = read_text(BOTTLE_ROOT / "system.reg")
desktop = section_value(user_reg, "Software\\\\Wine\\\\Explorer\\\\Desktops", "Shandalar1440")
app_version = section_value(user_reg, "Software\\\\Wine\\\\AppDefaults\\\\Shandalar.exe", "Version")
app_desktop = section_value(user_reg, "Software\\\\Wine\\\\AppDefaults\\\\Shandalar.exe\\\\Explorer", "Desktop")

print("# Manual Gameplay Baseline")
print()
print("Paste this into docs/manual-gameplay-verification.md before a visible test run, then replace result fields with pass/fail evidence.")
print()
print("## Environment")
print()
print("| Field | Value |")
print("| --- | --- |")
for left, right in [
    ("Generated", datetime.now().strftime("%Y-%m-%d %H:%M:%S")),
    ("Repo path", str(REPO)),
    ("Git branch", branch),
    ("Git commit", commit),
    ("Git status", status),
    ("Platform", platform.platform()),
    ("Bottle name", BOTTLE),
    ("Bottle path", str(BOTTLE_ROOT) if BOTTLE_ROOT.exists() else f"missing: {BOTTLE_ROOT}"),
    ("Bottle Windows version", system_windows_version(system_reg)),
    ("CrossOver wine helper", wine_version()),
    ("App-default Version", app_version),
    ("App-default Desktop", app_desktop),
    ("Virtual desktop", f"Shandalar1440={desktop}" if desktop != "not found" else "not found"),
    ("Primary working directory", "C:\\Shandalar"),
    ("Primary target", "C:\\Shandalar\\Shandalar.exe"),
]:
    print(markdown_row(left, right))

print()
print("## Launch Commands")
print()
print("```sh")
print(f'/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle {BOTTLE} --workdir "C:\\Shandalar" "C:\\Shandalar\\Shandalar.exe"')
print(f'/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle {BOTTLE} --workdir "C:\\Shandalar" "C:\\Shandalar\\Magic.exe"')
print(f'/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle {BOTTLE} --workdir "C:\\Shandalar\\Program" "C:\\Shandalar\\Program\\Magic.exe"')
print("```")

print()
print("## Hashes")
print()
print("| Target | SHA-256 |")
print("| --- | --- |")
for label, path in HASH_TARGETS:
    print(markdown_row(label, file_sha(path)))
PY
