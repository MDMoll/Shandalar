#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: tools/check-build-prereqs.sh [--report-only]

Read-only preflight for Shandalar source/build work. It checks known local
toolchain and source-tree prerequisites without building, copying, launching,
or modifying runtime files.

Options:
  --report-only  Print failures but exit 0.
  -h, --help     Show this help.
USAGE
}

report_only=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --report-only)
      report_only=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      printf 'Unknown option: %s\n\n' "$1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

failures=0
warnings=0

emit() {
  local status="$1"
  local area="$2"
  local check="$3"
  local detail="$4"
  printf '%s\t%s\t%s\t%s\n' "$status" "$area" "$check" "$detail"
}

fail() {
  failures=$((failures + 1))
  emit "fail" "$@"
}

warn() {
  warnings=$((warnings + 1))
  emit "warn" "$@"
}

ok() {
  emit "ok" "$@"
}

check_tool() {
  local tool="$1"
  local area="$2"
  local requirement="$3"
  local path
  path="$(command -v "$tool" 2>/dev/null || true)"
  if [[ -n "$path" ]]; then
    ok "$area" "$tool" "$path"
  elif [[ "$requirement" == "required" ]]; then
    fail "$area" "$tool" "not found"
  else
    warn "$area" "$tool" "not found"
  fi
}

check_file() {
  local path="$1"
  local area="$2"
  local requirement="$3"
  local detail="$4"
  if [[ -f "$path" ]]; then
    ok "$area" "$path" "$detail"
  elif [[ "$requirement" == "required" ]]; then
    fail "$area" "$path" "missing; $detail"
  else
    warn "$area" "$path" "missing; $detail"
  fi
}

check_same_file() {
  local left="$1"
  local right="$2"
  local area="$3"
  local detail="$4"
  if [[ -f "$left" && -f "$right" ]]; then
    if cmp -s "$left" "$right"; then
      ok "$area" "$left == $right" "$detail"
    else
      fail "$area" "$left == $right" "files differ; $detail"
    fi
  fi
}

printf 'status\tarea\tcheck\tdetail\n'
ok "repo" "working-directory" "$repo_root"

check_file "src/Makefile" "top-level-src" "required" "top-level source makefile"
check_file "Program/src/Makefile" "program-src" "required" "Program source makefile"
check_file "src/card_id.h" "top-level-src" "required" "top-level card id header required by src/Makefile-targets"
check_file "Program/src/card_id.h" "program-src" "required" "Program source snapshot has generated card id header"
check_same_file "src/card_id.h" "Program/src/card_id.h" "source-provenance" "top-level header is intentionally restored from the Program source snapshot"

check_tool "make" "dry-run-build" "required"
check_tool "perl" "tooling" "required"
check_tool "python3" "tooling" "required"
check_tool "gcc" "real-build-toolchain" "required"
check_tool "g++" "real-build-toolchain" "required"
check_tool "yasm" "real-build-toolchain" "required"
check_tool "dlltool" "real-build-toolchain" "required"
check_tool "objcopy" "real-build-toolchain" "required"
check_tool "windres" "real-build-toolchain" "required"

if [[ -d src && -d Program/src ]]; then
  warn "source-provenance" "src-vs-Program/src" "both source snapshots exist; audit found divergence, so source-to-runtime claims need explicit provenance"
fi

if [[ -f src/build.pl ]]; then
  if perl -c src/build.pl >/dev/null 2>&1; then
    ok "tooling" "src/build.pl" "Perl syntax check passed"
  else
    fail "tooling" "src/build.pl" "Perl syntax check failed"
  fi
fi

emit "summary" "result" "failures" "$failures"
emit "summary" "result" "warnings" "$warnings"

if [[ "$failures" -gt 0 && "$report_only" -eq 0 ]]; then
  exit 1
fi

exit 0
