#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

include_crossover=0
crossover_bottle="MTG"
verify_bundle_import=0
verify_artifacts=0
temp_paths=()

cleanup_temp_paths() {
  local path
  set +u
  for path in "${temp_paths[@]}"; do
    [ -n "$path" ] && rm -rf "$path"
  done
  set -u
}
trap cleanup_temp_paths EXIT

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
  --verify-artifacts      Verify the default /private/tmp bundle and patch
                          artifacts for the current branch tip.
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
    --verify-artifacts)
      verify_artifacts=1
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
branch="$(git branch --show-current)"
[ -n "$branch" ] || fail "current checkout is detached; this verifier expects a branch"
safe_branch="$(printf '%s' "$branch" | tr '/: ' '---')"
source_ref="refs/heads/$branch"
dest_ref="refs/heads/$branch"
default_bundle_path="/private/tmp/${safe_branch}-${short_sha}.bundle"
default_patch_path="/private/tmp/${safe_branch}-${short_sha}.patch"

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

share_status="$(tools/print-share-status.sh)"
expect_share_status() {
  local needle="$1"
  local message="$2"

  printf '%s\n' "$share_status" | grep -F -q "$needle" || fail "$message"
}

branch_delta="$(tools/list-branch-delta.sh)"
branch_delta_count="$(printf '%s\n' "$branch_delta" | awk 'NR > 1 {count++} END {print count+0}')"
branch_delta_other_count="$(printf '%s\n' "$branch_delta" | awk -F '\t' 'NR > 1 && $4 == "other" {count++} END {print count+0}')"
expect_share_status "| Commit | \`$short_sha\`" "share status does not report commit $short_sha"
expect_share_status "| Git status | clean |" "share status does not report clean status"
if [ -f security-scan-results.tsv ]; then
  expect_share_status "| Ignored local scan evidence | present as ignored local evidence |" "share status does not report local scan evidence"
else
  expect_share_status "| Ignored local scan evidence | absent |" "share status does not report absent local scan evidence"
fi
if git rev-parse --abbrev-ref --symbolic-full-name '@{u}' >/dev/null 2>&1; then
  upstream="$(git rev-parse --abbrev-ref --symbolic-full-name '@{u}')"
  expect_share_status "| Upstream | \`$upstream\` |" "share status does not report upstream $upstream"
  if [ "$(git rev-parse HEAD)" = "$(git rev-parse "$upstream")" ]; then
    expect_share_status "| GitHub push | Pushed; local \`HEAD\` matches \`$upstream\` at \`$short_sha\`. |" "share status does not report pushed upstream state"
  fi
fi
expect_share_status "| Branch delta rows | $branch_delta_count | \`tools/list-branch-delta.sh\`; rows classified as \`other\`: $branch_delta_other_count |" "share status does not report current branch-delta count"
if [ -f security-scan-results.tsv ] && tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all >/dev/null 2>&1; then
  security_inventory_note="\`tools/list-security-scan-targets.sh\`; local scanner results validate for all current targets"
else
  security_inventory_note="\`tools/list-security-scan-targets.sh\`; scanner still needs to be run separately"
fi
expect_share_status "| Security scan targets | $security_target_count | $security_inventory_note |" "share status does not report current security-target count"
expect_share_status "$default_bundle_path" "share status missing current bundle path"
expect_share_status "Default Handoff Artifact Hashes" "share status missing artifact hash section"
expect_share_status "Security Scan Results" "share status missing security scan results section"
expect_share_status "| Results file | \`security-scan-results.tsv\` |" "share status missing security scan results file"
expect_share_status "verify-security-scan-results.sh --results security-scan-results.tsv --require-all" "share status missing security scan validation command"
expect_share_status "Manual Gameplay Results" "share status missing manual gameplay results section"
expect_share_status "| Results doc | \`docs/manual-gameplay-verification.md\` |" "share status missing manual gameplay doc"
expect_share_status "verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md" "share status missing manual gameplay validation command"
expect_share_status "verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md --allow-incomplete --show-missing" "share status missing manual gameplay gap command"
if [ ! -f security-scan-results.tsv ]; then
  security_results_status="missing; no scanner results recorded"
elif tools/verify-security-scan-results.sh --results security-scan-results.tsv >/dev/null 2>&1; then
  if tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all >/dev/null 2>&1; then
    security_results_status="present; all $security_target_count tracked target rows validate"
  else
    security_results_count="$(awk -F '\t' 'NR > 1 && NF > 0 {count++} END {print count+0}' security-scan-results.tsv)"
    security_results_status="present; $security_results_count recorded row(s) validate, but full coverage is incomplete"
  fi
else
  security_results_status="present; validation FAILED"
fi
manual_gameplay_status="$(tools/verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md --allow-incomplete | sed 's/^ok: //')"
expect_share_status "| Validation | $security_results_status |" "share status does not report current security scan result validation status"
expect_share_status "| Validation | $manual_gameplay_status |" "share status does not report current manual gameplay validation status"
if [ -f "$default_bundle_path" ]; then
  bundle_sha="$(shasum -a 256 "$default_bundle_path" | awk '{print $1}')"
  expect_share_status "| Git bundle | \`$bundle_sha\` |" "share status does not report current bundle hash $bundle_sha"
else
  expect_share_status "| Git bundle | \`missing\` |" "share status does not report missing bundle hash"
fi
if [ -f "$default_patch_path" ]; then
  patch_sha="$(shasum -a 256 "$default_patch_path" | awk '{print $1}')"
  expect_share_status "| Binary patch | \`$patch_sha\` |" "share status does not report current patch hash $patch_sha"
else
  expect_share_status "| Binary patch | \`missing\` |" "share status does not report missing patch hash"
fi
expect_share_status "Manual gameplay" "share status missing manual gameplay gate"
expect_share_status "tools/verify-final-share-gates.sh" "share status missing strict final verifier gate"
pass "share status report is current"

cleanup_dest="/private/tmp/shandalar-cleanup-test-dryrun-${short_sha}-$$"
cleanup_dry_run="$(tools/create-cleanup-test-copy.sh --dry-run --skip-verify "$cleanup_dest" 2>&1)"
printf '%s\n' "$cleanup_dry_run" | grep -q "without .git: $cleanup_dest" || fail "cleanup copy dry-run does not report default no-.git copy"
cleanup_with_git_dest="/private/tmp/shandalar-cleanup-test-with-git-dryrun-${short_sha}-$$"
cleanup_with_git_dry_run="$(tools/create-cleanup-test-copy.sh --dry-run --include-git --skip-verify "$cleanup_with_git_dest" 2>&1)"
printf '%s\n' "$cleanup_with_git_dry_run" | grep -q "including .git: $cleanup_with_git_dest" || fail "cleanup copy dry-run does not report --include-git copy"
pass "cleanup test copy dry-runs passed"

bundle_dest="/private/tmp/shandalar-handoff-dryrun-${short_sha}-$$.bundle"
bundle_dry_run="$(tools/create-git-handoff-bundle.sh --dry-run --skip-verify --dest "$bundle_dest")"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver checksum command" || fail "bundle dry-run missing receiver checksum command"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver verify command:" || fail "bundle dry-run missing receiver verify command"
printf '%s\n' "$bundle_dry_run" | grep -q "Receiver fetch command:" || fail "bundle dry-run missing receiver fetch command"
pass "Git bundle dry-run passed"

patch_dest="/private/tmp/shandalar-patch-dryrun-${short_sha}-$$.patch"
patch_dry_run="$(tools/create-patch-package.sh --dry-run --skip-verify --dest "$patch_dest")"
printf '%s\n' "$patch_dry_run" | grep -q "Receiver checksum command" || fail "patch dry-run missing receiver checksum command"
printf '%s\n' "$patch_dry_run" | grep -q "Receiver check command" || fail "patch dry-run missing receiver check command"
printf '%s\n' "$patch_dry_run" | grep -q "Receiver apply command" || fail "patch dry-run missing receiver apply command"
pass "Git patch dry-run passed"

if [ "$verify_bundle_import" = "1" ]; then
  real_bundle="/private/tmp/shandalar-handoff-import-${short_sha}-$$.bundle"
  temp_paths+=("$real_bundle" "${real_bundle}.sha256")
  temp_root="$(mktemp -d "/private/tmp/shandalar-bundle-import-${short_sha}.XXXXXX")"
  temp_paths+=("$temp_root")
  temp_repo="$temp_root/repo"

  tools/create-git-handoff-bundle.sh --skip-verify --dest "$real_bundle" >/dev/null
  [ -s "${real_bundle}.sha256" ] || fail "bundle checksum sidecar was not created: ${real_bundle}.sha256"
  (
    cd "$(dirname "$real_bundle")"
    shasum -a 256 -c "$(basename "${real_bundle}.sha256")" >/dev/null
  )
  git clone --single-branch --branch master --no-checkout "$repo_root" "$temp_repo" >/dev/null
  git -C "$temp_repo" bundle verify "$real_bundle" >/dev/null
  git -C "$temp_repo" fetch "$real_bundle" "$source_ref:$dest_ref" >/dev/null
  imported_sha="$(git -C "$temp_repo" rev-parse --short "$dest_ref")"
  [ "$imported_sha" = "$short_sha" ] || fail "bundle import resolved $imported_sha, expected $short_sha"
  pass "Git bundle import verified in $temp_repo using $real_bundle"
else
  pass "Git bundle import verification skipped; pass --verify-bundle-import for a real import test"
fi

if [ "$verify_artifacts" = "1" ]; then
  tools/verify-handoff-artifacts.sh
  pass "default handoff artifacts verified"
else
  pass "default handoff artifact verification skipped; pass --verify-artifacts after creating them"
fi

if [ "$include_crossover" = "1" ]; then
  tools/verify-crossover-mtg-state.sh "$crossover_bottle"
  pass "CrossOver bottle-state verifier passed for $crossover_bottle"
else
  pass "CrossOver bottle-state verifier skipped; pass --include-crossover on this Mac"
fi

printf 'Handoff readiness checks passed for %s.\n' "$short_sha"
