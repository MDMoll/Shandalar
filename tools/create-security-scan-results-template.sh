#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

output_path=""
replace=0

usage() {
  cat <<'EOF'
Usage: tools/create-security-scan-results-template.sh [options]

Writes a TSV template for recording real scanner results against the current
tracked scan target inventory. This does not run a scanner and the placeholders
must be replaced before tools/verify-security-scan-results.sh can pass.

Options:
  --output PATH  Write the template to PATH instead of stdout.
  --replace      Allow replacing an existing regular output file.
  -h, --help     Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output)
      shift
      [ "$#" -gt 0 ] || fail "--output requires a value"
      output_path="$1"
      ;;
    --replace)
      replace=1
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

write_template() {
  tools/list-security-scan-targets.sh |
    awk -F '\t' 'BEGIN {OFS="\t"; print "path", "sha256", "scanner", "version", "date", "result", "notes"} NR > 1 {print $1, $4, "Needs testing", "Needs testing", "Needs testing", "Needs testing", "Fill after a real scanner pass; target kind=" $2 ", bytes=" $3}'
}

if [ -z "$output_path" ]; then
  write_template
  exit 0
fi

if [ -e "$output_path" ] && [ "$replace" != "1" ]; then
  fail "$output_path already exists; pass --replace to overwrite a regular file"
fi

if [ -e "$output_path" ] && [ ! -f "$output_path" ]; then
  fail "$output_path exists but is not a regular file"
fi

write_template > "$output_path"
printf 'Wrote security scan results template: %s\n' "$output_path"
