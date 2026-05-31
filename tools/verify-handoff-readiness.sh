#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

include_crossover=0
crossover_bottle="MTG"
verify_bundle_import=0

usage() {
  cat <<'EOF'
Usage: tools/verify-handoff-readiness.sh [options]

Runs the non-gameplay handoff checks that should pass before sharing this
branch or handing it off as a Git bundle. It does not launch the game, run a
malware scanner, or prove gameplay.

Options:
  --include-crossover     Also run the local MTG CrossOver bottle-state check.
  --crossover-bottle NAME Bottle name for --include-crossover. Defaults to MTG.
  --verify-bundle-import  Create a real bundle and import it into a disposable
                          master-only clone under /private/tmp.
  -h, --help              Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --include-crossover)
      include_crossover=1
      ;;
    --crossover-bottle)
      shift
      [ "$#" -gt 0 ] || fail "--crossover-bottle requires a value"
      crossover_bottle="$1"
      ;;
    --verify-bundle-import)
      verify_bundle_import=1
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

short_sha="$(git rev-parse --short HEAD)"

tools/verify-share-readiness.sh
pass "base share-readiness verifier passed"

manual_baseline="$(tools/print-manual-gameplay-baseline.sh)"
printf '%s\n' "$manual_baseline" | grep -q "| Git commit | $short_sha |" || fail "manual gameplay baseline does not report commit $short_sha"
printf '%s\n' "$manual_baseline" | grep -q "| Git status | clean |" || fail "manual gameplay baseline does not report clean status"
printf '%s\n' "$manual_baseline" | grep -q "| Primary target | C:\\\\Shandalar\\\\Shandalar.exe |" || fail "manual gameplay baseline missing primary target"
pass "manual gameplay baseline is current"

security_baseline="$(tools/print-security-scan-baseline.sh)"
security_target_count="$(tools/list-security-scan-targets.sh | awk 'NR > 1 {count++} END {print count+0}')"
printf '%s\n' "$security_baseline" | grep -q "| Git commit | $short_sha |" || fail "security scan baseline does not report commit $short_sha"
printf '%s\n' "$security_baseline" | grep -q "| Git status | clean |" || fail "security scan baseline does not report clean status"
printf '%s\n' "$security_baseline" | grep -q "| Tracked scan targets | $security_target_count |" || fail "security scan baseline does not report $security_target_count targets"
pass "security scan baseline is current"

cleanup_dest="/private/tmp/shandalar-cleanup-test-dryrun-${short_sha}-$$"
tools/create-cleanup-test-copy.sh --dry-run --skip-verify "$cleanup_dest" >/dev/null
pass "cleanup test copy dry-run passed"

bundle_dest="/private/tmp/shandalar-handoff-dryrun-${short_sha}-$$.bundle"
bundle_dry_run="$(tools/create-git-handoff-bundle.sh --dry-run --skip-verify --dest "$bundle_dest")"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver checksum command" || fail "bundle dry-run missing receiver checksum command"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver verify command:" || fail "bundle dry-run missing receiver verify command"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver fetch command:" || fail "bundle dry-run missing receiver fetch command"
pass "Git bundle dry-run passed"

if [ "$verify_bundle_import" = "1" ]; then
  real_bundle="/private/tmp/shandalar-handoff-import-${short_sha}-$$.bundle"
  temp_root="$(mktemp -d "/private/tmp/shandalar-bundle-import-${short_sha}.XXXXXX")"
  temp_repo="$temp_root/repo"

  tools/create-git-handoff-bundle.sh --skip-verify --dest "$real_bundle" >/dev/null
  [ -s "${real_bundle}.sha256" ] || fail "bundle checksum sidecar was not created: ${real_bundle}.sha256"
  (
    cd "$(dirname "$real_bundle")"
    shasum -a 256 -c "$(basename "${real_bundle}.sha256")" >/dev/null
  )
  git clone --single-branch --branch master --no-checkout "$repo_root" "$temp_repo" >/dev/null
  git -C "$temp_repo" bundle verify "$real_bundle" >/dev/null
  git -C "$temp_repo" fetch "$real_bundle" "refs/heads/codex/shandalar-crossover-updates:refs/heads/codex/shandalar-crossover-updates" >/dev/null
  imported_sha="$(git -C "$temp_repo" rev-parse --short codex/shandalar-crossover-updates)"
  [ "$imported_sha" = "$short_sha" ] || fail "bundle import resolved $imported_sha, expected $short_sha"
  pass "Git bundle import verified in $temp_repo using $real_bundle"
else
  pass "Git bundle import verification skipped; pass --verify-bundle-import for a real import test"
fi

if [ "$include_crossover" = "1" ]; then
  tools/verify-crossover-mtg-state.sh "$crossover_bottle"
  pass "CrossOver bottle-state verifier passed for $crossover_bottle"
else
  pass "CrossOver bottle-state verifier skipped; pass --include-crossover on this Mac"
fi

printf 'Handoff readiness checks passed for %s.\n' "$short_sha"
