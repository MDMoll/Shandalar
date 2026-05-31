#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

results_path="security-scan-results.tsv"
target_path=""
all_current_targets=0
scanner=""
version=""
scan_date=""
scan_result=""
notes=""
replace_row=0
dry_run=0
confirmed_real_scan=0

usage() {
  cat <<'EOF'
Usage: tools/record-security-scan-result.sh [options]

Records real scanner evidence in security-scan-results.tsv for the current
tracked scan target inventory. This does not run a scanner and does not decide
whether a finding is safe.

Examples:
  tools/record-security-scan-result.sh --confirmed-real-scan --path Shandalar.exe \
    --scanner "Windows Defender" --version "1.407.1234.0" --date 2026-05-31 \
    --result "Clean" --notes "MpCmdRun.exe custom scan completed"

  tools/record-security-scan-result.sh --confirmed-real-scan --all-current-targets --replace-row \
    --scanner "ClamAV" --version "1.3.1/27200" --date 2026-05-31 \
    --result "Clean" --notes "clamscan -r . completed with no infected files"

Options:
  --results PATH            Results TSV to update. Defaults to security-scan-results.tsv.
  --path PATH               Record one current tracked scan target.
  --all-current-targets     Record the same scanner result for every current tracked scan target.
  --scanner NAME            Scanner name used for the real scan.
  --version VERSION         Scanner or definition version.
  --date YYYY-MM-DD         Date the scan was run.
  --result RESULT           Scanner result text, such as Clean or Detected.
  --notes NOTES             Command, report path, or other evidence note.
  --replace-row             Allow replacing an existing row for the target path.
  --confirmed-real-scan     Required acknowledgement that an actual scanner was run.
  --dry-run                 Print the updated TSV instead of writing the file.
  -h, --help                Show this help.
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
    --path)
      shift
      [ "$#" -gt 0 ] || fail "--path requires a value"
      target_path="$1"
      ;;
    --all-current-targets)
      all_current_targets=1
      ;;
    --scanner)
      shift
      [ "$#" -gt 0 ] || fail "--scanner requires a value"
      scanner="$1"
      ;;
    --version)
      shift
      [ "$#" -gt 0 ] || fail "--version requires a value"
      version="$1"
      ;;
    --date)
      shift
      [ "$#" -gt 0 ] || fail "--date requires a value"
      scan_date="$1"
      ;;
    --result)
      shift
      [ "$#" -gt 0 ] || fail "--result requires a value"
      scan_result="$1"
      ;;
    --notes)
      shift
      [ "$#" -gt 0 ] || fail "--notes requires a value"
      notes="$1"
      ;;
    --replace-row)
      replace_row=1
      ;;
    --confirmed-real-scan)
      confirmed_real_scan=1
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

[ "$confirmed_real_scan" = "1" ] || fail "pass --confirmed-real-scan only after an actual scanner run"

if [ -n "$target_path" ] && [ "$all_current_targets" = "1" ]; then
  fail "choose either --path or --all-current-targets, not both"
fi

if [ -z "$target_path" ] && [ "$all_current_targets" != "1" ]; then
  fail "choose --path or --all-current-targets"
fi

for required_name in scanner version date result notes; do
  case "$required_name" in
    scanner) required_value="$scanner" ;;
    version) required_value="$version" ;;
    date) required_value="$scan_date" ;;
    result) required_value="$scan_result" ;;
    notes) required_value="$notes" ;;
  esac
  [ -n "$required_value" ] || fail "--$required_name is required"
done

case "$scanner$version$scan_date$scan_result$notes" in
  *$'\n'*|*$'\r'*|*$'\t'*)
    fail "metadata values must be single TSV cells"
    ;;
esac

case "$scanner$version$scan_date$scan_result$notes" in
  *"Needs testing"*|*"needs testing"*|*"TBD"*|*"tbd"*)
    fail "metadata values must not contain placeholder text"
    ;;
esac

case "$scan_date" in
  [0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])
    ;;
  *)
    fail "--date must use YYYY-MM-DD"
    ;;
esac

export SHANDALAR_SCAN_RESULTS="$results_path"
export SHANDALAR_SCAN_PATH="$target_path"
export SHANDALAR_SCAN_ALL="$all_current_targets"
export SHANDALAR_SCAN_SCANNER="$scanner"
export SHANDALAR_SCAN_VERSION="$version"
export SHANDALAR_SCAN_DATE="$scan_date"
export SHANDALAR_SCAN_RESULT="$scan_result"
export SHANDALAR_SCAN_NOTES="$notes"
export SHANDALAR_SCAN_REPLACE="$replace_row"
export SHANDALAR_SCAN_DRY_RUN="$dry_run"

python3 - <<'PY'
from pathlib import Path
import os
import subprocess
import sys

expected_header = ["path", "sha256", "scanner", "version", "date", "result", "notes"]

results_path = Path(os.environ["SHANDALAR_SCAN_RESULTS"])
target_path = os.environ["SHANDALAR_SCAN_PATH"]
all_current_targets = os.environ["SHANDALAR_SCAN_ALL"] == "1"
replace_row = os.environ["SHANDALAR_SCAN_REPLACE"] == "1"
dry_run = os.environ["SHANDALAR_SCAN_DRY_RUN"] == "1"

new_metadata = {
    "scanner": os.environ["SHANDALAR_SCAN_SCANNER"],
    "version": os.environ["SHANDALAR_SCAN_VERSION"],
    "date": os.environ["SHANDALAR_SCAN_DATE"],
    "result": os.environ["SHANDALAR_SCAN_RESULT"],
    "notes": os.environ["SHANDALAR_SCAN_NOTES"],
}

target_rows = subprocess.check_output(["tools/list-security-scan-targets.sh"], text=True).splitlines()
target_header = target_rows[0].split("\t") if target_rows else []
if target_header != ["path", "kind", "bytes", "sha256"]:
    print(f"FAIL: unexpected scan target header: {target_header}", file=sys.stderr)
    sys.exit(1)

target_order = []
target_sha = {}
for line in target_rows[1:]:
    path, _kind, _byte_count, sha256 = line.split("\t")
    target_order.append(path)
    target_sha[path] = sha256

if all_current_targets:
    update_paths = list(target_order)
else:
    if target_path not in target_sha:
        print(f"FAIL: path is not in current scan target inventory: {target_path}", file=sys.stderr)
        sys.exit(1)
    update_paths = [target_path]

rows = {}
if results_path.exists():
    lines = results_path.read_text(encoding="utf-8", errors="replace").splitlines()
    if lines:
        header = lines[0].split("\t")
        if header != expected_header:
            print(f"FAIL: unexpected scan results header in {results_path}: {header}", file=sys.stderr)
            sys.exit(1)

        for line_no, line in enumerate(lines[1:], start=2):
            if not line.strip():
                continue
            cells = line.split("\t")
            if len(cells) != len(expected_header):
                print(
                    f"FAIL: {results_path}:{line_no}: expected {len(expected_header)} tab-separated fields, got {len(cells)}",
                    file=sys.stderr,
                )
                sys.exit(1)
            row = dict(zip(expected_header, cells))
            path = row["path"]
            if path in rows:
                print(f"FAIL: duplicate scan result path already present: {path}", file=sys.stderr)
                sys.exit(1)
            if path not in target_sha:
                print(f"FAIL: existing row path is not in current scan target inventory: {path}", file=sys.stderr)
                sys.exit(1)
            rows[path] = row
    else:
        rows = {}

existing_updates = sorted(path for path in update_paths if path in rows)
if existing_updates and not replace_row:
    sample = ", ".join(existing_updates[:10])
    if len(existing_updates) > 10:
        sample += f", ... ({len(existing_updates)} existing total)"
    print(f"FAIL: existing scan result row(s) would be replaced; pass --replace-row: {sample}", file=sys.stderr)
    sys.exit(1)

for path in update_paths:
    rows[path] = {
        "path": path,
        "sha256": target_sha[path],
        **new_metadata,
    }

ordered_paths = [path for path in target_order if path in rows]
extra_paths = sorted(set(rows) - set(target_order))
if extra_paths:
    print(f"FAIL: existing rows outside current target inventory: {', '.join(extra_paths[:10])}", file=sys.stderr)
    sys.exit(1)

output_lines = ["\t".join(expected_header)]
for path in ordered_paths:
    row = rows[path]
    output_lines.append("\t".join(row[column] for column in expected_header))

output = "\n".join(output_lines) + "\n"
if dry_run:
    sys.stdout.write(output)
else:
    results_path.write_text(output, encoding="utf-8")
    plural = "rows" if len(update_paths) != 1 else "row"
    print(f"updated {results_path}: {len(update_paths)} {plural}")
    print(f"run: tools/verify-security-scan-results.sh --results {results_path}")
    if all_current_targets:
        print(f"run: tools/verify-security-scan-results.sh --results {results_path} --require-all")
PY
