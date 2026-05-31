#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

results_path="security-scan-results.tsv"
require_all=0
allow_missing=0

usage() {
  cat <<'EOF'
Usage: tools/verify-security-scan-results.sh [options]

Validates a local security scan results TSV against the current tracked scan
target inventory. This does not run a scanner.

Expected TSV header:
  path	sha256	scanner	version	date	result	notes

Options:
  --results PATH   Results TSV to validate. Defaults to security-scan-results.tsv.
  --require-all    Require one valid row for every tracked scan target.
  --allow-missing  Exit successfully if the results file is absent.
  -h, --help       Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --results)
      shift
      [ "$#" -gt 0 ] || fail "--results requires a value"
      results_path="$1"
      ;;
    --require-all)
      require_all=1
      ;;
    --allow-missing)
      allow_missing=1
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

if [ ! -f "$results_path" ]; then
  if [ "$allow_missing" = "1" ]; then
    printf 'ok: security scan results file is absent: %s\n' "$results_path"
    exit 0
  fi
  fail "security scan results file is absent: $results_path"
fi

target_inventory="$(tools/list-security-scan-targets.sh)"
export SHANDALAR_SCAN_RESULTS="$results_path"
export SHANDALAR_SCAN_REQUIRE_ALL="$require_all"
export SHANDALAR_SCAN_TARGETS="$target_inventory"

python3 - <<'PY'
from pathlib import Path
import os
import sys

expected_header = ["path", "sha256", "scanner", "version", "date", "result", "notes"]
results_path = Path(os.environ["SHANDALAR_SCAN_RESULTS"])
require_all = os.environ["SHANDALAR_SCAN_REQUIRE_ALL"] == "1"

target_rows = os.environ["SHANDALAR_SCAN_TARGETS"].splitlines()
target_header = target_rows[0].split("\t") if target_rows else []
if target_header != ["path", "kind", "bytes", "sha256"]:
    print(f"FAIL: unexpected scan target header: {target_header}", file=sys.stderr)
    sys.exit(1)

targets = {}
for line in target_rows[1:]:
    path, kind, byte_count, sha256 = line.split("\t")
    targets[path] = {
        "kind": kind,
        "bytes": int(byte_count),
        "sha256": sha256,
    }

lines = results_path.read_text(encoding="utf-8", errors="replace").splitlines()
if not lines:
    print(f"FAIL: empty scan results file: {results_path}", file=sys.stderr)
    sys.exit(1)

header = lines[0].split("\t")
if header != expected_header:
    print(
        "FAIL: unexpected scan results header: "
        + repr(header)
        + "; expected "
        + repr(expected_header),
        file=sys.stderr,
    )
    sys.exit(1)

seen = set()
failures = []
placeholder_values = {"", "Needs testing", "needs testing", "TBD", "tbd"}

for line_no, line in enumerate(lines[1:], start=2):
    if not line.strip():
        continue

    cells = line.split("\t")
    if len(cells) != len(expected_header):
        failures.append(f"{results_path}:{line_no}: expected {len(expected_header)} tab-separated fields, got {len(cells)}")
        continue

    row = dict(zip(expected_header, cells))
    path = row["path"]
    target = targets.get(path)
    if target is None:
        failures.append(f"{results_path}:{line_no}: path is not in current scan target inventory: {path}")
        continue

    if path in seen:
        failures.append(f"{results_path}:{line_no}: duplicate scan result path: {path}")
        continue

    seen.add(path)

    if row["sha256"] != target["sha256"]:
        failures.append(
            f"{results_path}:{line_no}: sha256 for {path} is {row['sha256']}, expected {target['sha256']}"
        )

    for field in ["scanner", "version", "date", "result"]:
        if row[field] in placeholder_values:
            failures.append(f"{results_path}:{line_no}: {field} is not a recorded scanner value for {path}")

if not seen:
    failures.append(f"{results_path}: contains no scan result rows")

if require_all:
    missing = sorted(set(targets) - seen)
    if missing:
        sample = ", ".join(missing[:10])
        if len(missing) > 10:
            sample += f", ... ({len(missing)} missing total)"
        failures.append(f"{results_path}: missing scan result rows for tracked targets: {sample}")

if failures:
    for failure in failures:
        print(f"FAIL: {failure}", file=sys.stderr)
    sys.exit(1)

mode = "all tracked scan targets" if require_all else "recorded scan result rows"
print(f"ok: validated {len(seen)} {mode} in {results_path}")
PY
