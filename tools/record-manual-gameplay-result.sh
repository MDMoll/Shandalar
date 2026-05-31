#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

doc_path="docs/manual-gameplay-verification.md"
field_name=""
field_value=""
test_id=""
test_result=""
dry_run=0

usage() {
  cat <<'EOF'
Usage: tools/record-manual-gameplay-result.sh [options]

Updates one field or one required test result in the manual gameplay
verification Markdown table. This does not run the game or decide whether a
test passed; it only records visible tester evidence in the expected place.

Examples:
  tools/record-manual-gameplay-result.sh --field Date --value 2026-05-31
  tools/record-manual-gameplay-result.sh --field "Bottle Windows version" --value win7
  tools/record-manual-gameplay-result.sh --test D2 --result "Fail: froze at post-combat Done; screenshot /path/to/freeze.png"
  tools/record-manual-gameplay-result.sh --test R3 --result "Pass: clicked declared attacker again before Done and it cleared"

Options:
  --doc PATH       Manual gameplay doc to update.
                   Defaults to docs/manual-gameplay-verification.md.
  --field FIELD    Environment field name to update.
  --value VALUE    Replacement value for --field.
  --test ID        Required gameplay test ID to update, such as S1, D2, or R3.
  --result RESULT  Replacement result for --test. Must start with Pass or Fail.
  --dry-run        Print the updated Markdown instead of writing the file.
  -h, --help       Show this help.
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
    --field)
      shift
      [ "$#" -gt 0 ] || fail "--field requires a value"
      field_name="$1"
      ;;
    --value)
      shift
      [ "$#" -gt 0 ] || fail "--value requires a value"
      field_value="$1"
      ;;
    --test)
      shift
      [ "$#" -gt 0 ] || fail "--test requires a value"
      test_id="$1"
      ;;
    --result)
      shift
      [ "$#" -gt 0 ] || fail "--result requires a value"
      test_result="$1"
      ;;
    --dry-run)
      dry_run=1
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

if [ -n "$field_name" ] && [ -n "$test_id" ]; then
  fail "choose either --field or --test, not both"
fi

if [ -n "$field_name" ]; then
  [ -n "$field_value" ] || fail "--field requires --value"
  [ -z "$test_result" ] || fail "--result can only be used with --test"
elif [ -n "$test_id" ]; then
  [ -n "$test_result" ] || fail "--test requires --result"
  [ -z "$field_value" ] || fail "--value can only be used with --field"
else
  fail "choose --field or --test"
fi

case "$field_value$test_result" in
  *$'\n'*|*$'\r'*)
    fail "values must be a single Markdown table cell"
    ;;
  *'|'*)
    fail "values must not contain a pipe character"
    ;;
esac

export SHANDALAR_MANUAL_DOC="$doc_path"
export SHANDALAR_MANUAL_FIELD="$field_name"
export SHANDALAR_MANUAL_VALUE="$field_value"
export SHANDALAR_MANUAL_TEST="$test_id"
export SHANDALAR_MANUAL_RESULT="$test_result"
export SHANDALAR_MANUAL_DRY_RUN="$dry_run"

python3 - <<'PY'
from pathlib import Path
import os
import re
import sys

doc_path = Path(os.environ["SHANDALAR_MANUAL_DOC"])
field = os.environ["SHANDALAR_MANUAL_FIELD"]
value = os.environ["SHANDALAR_MANUAL_VALUE"]
test_id = os.environ["SHANDALAR_MANUAL_TEST"]
result = os.environ["SHANDALAR_MANUAL_RESULT"]
dry_run = os.environ["SHANDALAR_MANUAL_DRY_RUN"] == "1"

required_test_sections = {
    "Core Shandalar Pass",
    "Duel Stability Pass",
    "Regression Scenarios",
}

required_test_ids = {
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
}

if test_id:
    if test_id not in required_test_ids:
        print(f"FAIL: unknown required manual gameplay test ID: {test_id}", file=sys.stderr)
        sys.exit(1)
    if not result.strip().lower().startswith(("pass", "passed", "fail", "failed")):
        print("FAIL: --result must start with Pass or Fail so the verifier can classify it", file=sys.stderr)
        sys.exit(1)

text = doc_path.read_text(encoding="utf-8")
lines = text.splitlines(keepends=True)
changed = False
current_section = None

def split_table(line):
    stripped = line.strip()
    if not stripped.startswith("|") or not stripped.endswith("|"):
        return None
    cells = [cell.strip() for cell in stripped.strip("|").split("|")]
    if cells and all(set(cell) <= {"-", ":"} for cell in cells):
        return None
    return cells

def render_table(cells, newline):
    return "| " + " | ".join(cells) + " |" + newline

for index, line in enumerate(lines):
    newline = "\n" if line.endswith("\n") else ""
    heading = re.match(r"^##\s+(.+?)\s*$", line.rstrip("\n"))
    if heading:
        current_section = heading.group(1)
        continue

    cells = split_table(line)
    if not cells:
        continue

    if field and current_section == "Test Environment Record":
        if len(cells) >= 2 and cells[0] == field:
            cells[1] = value
            lines[index] = render_table(cells, newline)
            changed = True
            break

    if test_id and current_section in required_test_sections:
        if len(cells) >= 4 and cells[0] == test_id:
            cells[3] = result
            lines[index] = render_table(cells, newline)
            changed = True
            break

if not changed:
    target = f"field {field!r}" if field else f"test {test_id!r}"
    print(f"FAIL: could not find {target} in {doc_path}", file=sys.stderr)
    sys.exit(1)

updated = "".join(lines)
if dry_run:
    sys.stdout.write(updated)
else:
    doc_path.write_text(updated, encoding="utf-8")
    if field:
        print(f"updated {doc_path}: {field}")
    else:
        print(f"updated {doc_path}: {test_id}")
    print("run: tools/verify-manual-gameplay-results.sh --allow-incomplete")
PY
