#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

doc_path="docs/manual-gameplay-verification.md"
allow_incomplete=0
show_missing=0

usage() {
  cat <<'EOF'
Usage: tools/verify-manual-gameplay-results.sh [options]

Checks the manual gameplay verification checklist. This does not launch the
game; it validates recorded visible-test evidence in Markdown.

Options:
  --doc PATH           Manual gameplay verification doc to check.
                      Defaults to docs/manual-gameplay-verification.md.
  --allow-incomplete  Print the current status and exit successfully even when
                      evidence is missing or failing.
  --show-missing      Also print the missing/placeholder fields and test IDs.
  -h, --help          Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --doc)
      shift
      [ "$#" -gt 0 ] || fail "--doc requires a value"
      doc_path="$1"
      ;;
    --allow-incomplete)
      allow_incomplete=1
      ;;
    --show-missing)
      show_missing=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      fail "unknown option: $1"
      ;;
  esac
  shift
done

[ -f "$doc_path" ] || fail "manual gameplay doc is absent: $doc_path"

export SHANDALAR_MANUAL_DOC="$doc_path"
export SHANDALAR_ALLOW_INCOMPLETE="$allow_incomplete"
export SHANDALAR_SHOW_MISSING="$show_missing"

python3 - <<'PY'
from pathlib import Path
import os
import re
import sys

doc_path = Path(os.environ["SHANDALAR_MANUAL_DOC"])
allow_incomplete = os.environ["SHANDALAR_ALLOW_INCOMPLETE"] == "1"
show_missing = os.environ["SHANDALAR_SHOW_MISSING"] == "1"
text = doc_path.read_text(encoding="utf-8")

required_env_fields = [
    "Date",
    "Tester",
    "Platform",
    "CrossOver or Wine version",
    "Bottle name",
    "Bottle Windows version",
    "Virtual desktop",
    "Working directory",
    "Command or shortcut target",
    "`Shandalar.exe` SHA-256",
    "`Magic.exe` SHA-256",
    "`ManalinkEh.dll` SHA-256",
]

required_tests = [
    "S1",
    "S2",
    "S3",
    "S4",
    "S5",
    "S6",
    "S7",
    "S8",
    "D1",
    "D2",
    "D3",
    "D4",
    "D5",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
]


def section_text(name: str) -> str:
    pattern = re.compile(rf"^## {re.escape(name)}\n(?P<body>.*?)(?=^## |\Z)", re.M | re.S)
    match = pattern.search(text)
    return match.group("body") if match else ""


def table_rows(section: str):
    rows = []
    for raw_line in section.splitlines():
        line = raw_line.strip()
        if not line.startswith("|") or not line.endswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip("|").split("|")]
        if not cells or all(set(cell) <= {"-", ":"} for cell in cells):
            continue
        rows.append(cells)
    return rows


def placeholder(value: str) -> bool:
    stripped = value.strip()
    lower = stripped.lower()
    return (
        not stripped
        or "Needs testing" in stripped
        or "automated baseline only" in lower
        or "visible gameplay not run" in lower
        or stripped in {"Windows / CrossOver / Wine"}
    )


env_rows = table_rows(section_text("Test Environment Record"))
env_values = {}
for row in env_rows:
    if len(row) < 2 or row[0] == "Field":
        continue
    env_values[row[0]] = row[1]

missing_env = [field for field in required_env_fields if field not in env_values]
placeholder_env = [
    field for field in required_env_fields if field in env_values and placeholder(env_values[field])
]

test_rows = {}
for section_name in ["Core Shandalar Pass", "Duel Stability Pass", "Regression Scenarios"]:
    for row in table_rows(section_text(section_name)):
        if len(row) < 4 or row[0] == "ID":
            continue
        if row[0] in required_tests:
            test_rows[row[0]] = row[3]

missing_tests = [test_id for test_id in required_tests if test_id not in test_rows]
placeholder_tests = [
    test_id for test_id in required_tests if test_id in test_rows and placeholder(test_rows[test_id])
]

recorded_tests = [
    test_id
    for test_id in required_tests
    if test_id in test_rows and not placeholder(test_rows[test_id])
]
passing_tests = [
    test_id for test_id in recorded_tests if test_rows[test_id].strip().lower().startswith(("pass", "passed"))
]
failing_tests = [
    test_id for test_id in recorded_tests if test_rows[test_id].strip().lower().startswith(("fail", "failed"))
]
unclear_tests = [
    test_id
    for test_id in recorded_tests
    if test_id not in passing_tests and test_id not in failing_tests
]

incomplete_count = len(missing_env) + len(placeholder_env) + len(missing_tests) + len(placeholder_tests)
failure_count = len(failing_tests) + len(unclear_tests)

if incomplete_count == 0 and failure_count == 0 and len(passing_tests) == len(required_tests):
    print(f"ok: manual gameplay evidence complete; {len(passing_tests)}/{len(required_tests)} required tests pass")
    sys.exit(0)

parts = []
if missing_env:
    parts.append(f"{len(missing_env)} environment field(s) missing")
if placeholder_env:
    parts.append(f"{len(placeholder_env)} environment field(s) need testing")
if missing_tests:
    parts.append(f"{len(missing_tests)} required test row(s) missing")
if placeholder_tests:
    parts.append(f"{len(placeholder_tests)} required test result(s) need testing")
if failing_tests:
    parts.append(f"{len(failing_tests)} required test result(s) recorded as failing")
if unclear_tests:
    parts.append(f"{len(unclear_tests)} required test result(s) recorded without Pass/Fail")

summary = "; ".join(parts) if parts else "manual gameplay evidence is incomplete"
message = f"manual gameplay incomplete: {summary}; {len(passing_tests)}/{len(required_tests)} required tests pass"

details = []
if missing_env:
    details.append(f"missing environment fields: {', '.join(missing_env)}")
if placeholder_env:
    details.append(f"environment fields needing evidence: {', '.join(placeholder_env)}")
if missing_tests:
    details.append(f"missing required test rows: {', '.join(missing_tests)}")
if placeholder_tests:
    details.append(f"required test results needing evidence: {', '.join(placeholder_tests)}")
if failing_tests:
    details.append(f"required test results recorded as failing: {', '.join(failing_tests)}")
if unclear_tests:
    details.append(f"required test results without Pass/Fail: {', '.join(unclear_tests)}")
if passing_tests:
    details.append(f"required tests passing: {', '.join(passing_tests)}")

if allow_incomplete:
    print(f"ok: {message}")
    if show_missing:
        for detail in details:
            print(detail)
    sys.exit(0)

print(f"FAIL: {message}", file=sys.stderr)
if show_missing:
    for detail in details:
        print(f"FAIL: {detail}", file=sys.stderr)
sys.exit(1)
PY
