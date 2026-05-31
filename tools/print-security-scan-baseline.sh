#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

python3 - <<'PY'
from collections import defaultdict
from datetime import datetime
from pathlib import Path
import subprocess


PRIORITY_TARGETS = [
    "Shandalar.exe",
    "Program/Shandalar.exe",
    "Magic.exe",
    "Program/Magic.exe",
    "FaceMaker.exe",
    "Program/FaceMaker.exe",
    "ManalinkEh.dll",
    "Program/ManalinkEh.dll",
    "DeckInjector.jar",
    "Mods/Util/7za.exe",
]


def run_text(args):
    try:
        result = subprocess.run(
            args,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
    except FileNotFoundError:
        return "not available"
    text = result.stdout.strip()
    return text or f"exit {result.returncode}, no output"


def markdown_row(*cells):
    return "| " + " | ".join(str(cell) for cell in cells) + " |"


def parse_targets():
    output = subprocess.check_output(["tools/list-security-scan-targets.sh"], text=True)
    rows = []
    for line in output.splitlines()[1:]:
        path, kind, byte_count, sha256 = line.split("\t")
        rows.append(
            {
                "path": path,
                "kind": kind,
                "bytes": int(byte_count),
                "sha256": sha256,
            }
        )
    return rows


targets = parse_targets()
by_kind = defaultdict(lambda: {"count": 0, "bytes": 0})
by_path = {row["path"]: row for row in targets}
for row in targets:
    by_kind[row["kind"]]["count"] += 1
    by_kind[row["kind"]]["bytes"] += row["bytes"]

branch = run_text(["git", "branch", "--show-current"])
commit = run_text(["git", "rev-parse", "--short", "HEAD"])
status = run_text(["git", "status", "--short", "--untracked-files=all"])
status = "clean" if status == "exit 0, no output" else status.replace("\n", "<br>")
repo_path = Path.cwd()

print("# Security Scan Baseline")
print()
print("This helper does not run a scanner. Paste this baseline into docs/security-scan.md after a real scanner pass, then replace placeholders with scanner/version/result evidence.")
print()
print("## Inventory")
print()
print("| Field | Value |")
print("| --- | --- |")
for left, right in [
    ("Generated", datetime.now().strftime("%Y-%m-%d %H:%M:%S")),
    ("Repo path", repo_path),
    ("Git branch", branch),
    ("Git commit", commit),
    ("Git status", status),
    ("Tracked scan targets", len(targets)),
]:
    print(markdown_row(left, right))

print()
print("## Target Summary")
print()
print("| Kind | Count | Bytes |")
print("| --- | ---: | ---: |")
for kind in sorted(by_kind):
    stats = by_kind[kind]
    print(markdown_row(kind, stats["count"], stats["bytes"]))

print()
print("## Priority Hashes")
print()
print("| Path | Kind | SHA-256 |")
print("| --- | --- | --- |")
for path in PRIORITY_TARGETS:
    row = by_path.get(path)
    if row:
        print(markdown_row(path, row["kind"], row["sha256"]))
    else:
        print(markdown_row(path, "missing", "missing"))

print()
print("## Commands To Run")
print()
print("```sh")
print("tools/list-security-scan-targets.sh > scan-targets.tsv")
print("tools/create-security-scan-results-template.sh --output security-scan-results.tsv")
print("clamscan -r . > clamscan-report.txt")
print('"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe" -Scan -ScanType 3 -File "C:\\path\\to\\Shandalar"')
print("tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all")
print("```")

print()
print("## Result Rows To Fill")
print()
print("| Date | Scanner/version | Path or scope | SHA-256 or inventory | Result | Notes |")
print("| --- | --- | --- | --- | --- | --- |")
print(markdown_row("Needs testing", "Needs testing", "Full tracked target inventory", f"{len(targets)} targets", "Needs testing", "No scanner was run by this helper."))
PY
