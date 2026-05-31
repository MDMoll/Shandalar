#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

base="master"
branch="$(git branch --show-current)"

usage() {
  cat <<'EOF'
Usage: tools/verify-handoff-artifacts.sh [options]

Verifies the default /private/tmp Git bundle and binary patch artifacts for the
current branch tip. This does not create artifacts; run
tools/create-git-handoff-bundle.sh and tools/create-patch-package.sh first.

Options:
  --base REF    Base ref expected by the incremental bundle and patch. Defaults to master.
  --branch REF  Branch/ref to verify. Defaults to the current branch.
  -h, --help    Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

verify_checksum() {
  local artifact="$1"
  local checksum="$2"

  [ -s "$artifact" ] || fail "missing artifact: $artifact"
  [ -s "$checksum" ] || fail "missing checksum sidecar: $checksum"

  (
    cd "$(dirname "$artifact")"
    shasum -a 256 -c "$(basename "$checksum")" >/dev/null
  )
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --base)
      shift
      [ "$#" -gt 0 ] || fail "--base requires a value"
      base="$1"
      ;;
    --branch)
      shift
      [ "$#" -gt 0 ] || fail "--branch requires a value"
      branch="$1"
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

[ -n "$branch" ] || fail "no branch/ref specified and current branch is empty"
git rev-parse --verify "$branch^{commit}" >/dev/null || fail "unknown branch/ref: $branch"
git rev-parse --verify "$base^{commit}" >/dev/null || fail "unknown base ref: $base"

status="$(git status --short --untracked-files=all)"
[ -z "$status" ] || fail "working tree is not clean"
pass "working tree is clean"

short_sha="$(git rev-parse --short "$branch")"
safe_branch="$(printf '%s' "$branch" | tr '/: ' '---')"
bundle_path="/private/tmp/${safe_branch}-${short_sha}.bundle"
bundle_checksum_path="${bundle_path}.sha256"
patch_path="/private/tmp/${safe_branch}-${short_sha}.patch"
patch_checksum_path="${patch_path}.sha256"
expected_tree="$(git rev-parse "$branch^{tree}")"
base_commit="$(git rev-parse "$base^{commit}")"

verify_checksum "$bundle_path" "$bundle_checksum_path"
pass "bundle checksum verified: $bundle_path"

git bundle verify "$bundle_path" >/dev/null
pass "bundle verifies: $bundle_path"

bundle_temp_root="$(mktemp -d "/private/tmp/shandalar-artifact-bundle-${short_sha}.XXXXXX")"
bundle_temp_repo="$bundle_temp_root/repo"
git clone --single-branch --branch "$base" --no-checkout "$repo_root" "$bundle_temp_repo" >/dev/null
git -C "$bundle_temp_repo" fetch "$bundle_path" "refs/heads/${branch}:refs/heads/${branch}" >/dev/null
imported_sha="$(git -C "$bundle_temp_repo" rev-parse --short "$branch")"
[ "$imported_sha" = "$short_sha" ] || fail "bundle import resolved $imported_sha, expected $short_sha"
pass "bundle imports current branch tip in $bundle_temp_repo"

verify_checksum "$patch_path" "$patch_checksum_path"
pass "patch checksum verified: $patch_path"

patch_temp_root="$(mktemp -d "/private/tmp/shandalar-artifact-patch-${short_sha}.XXXXXX")"
patch_temp_repo="$patch_temp_root/repo"
git clone --no-checkout "$repo_root" "$patch_temp_repo" >/dev/null
git -C "$patch_temp_repo" read-tree "$base_commit"
git -C "$patch_temp_repo" apply --cached --binary --whitespace=nowarn "$patch_path"
actual_tree="$(git -C "$patch_temp_repo" write-tree)"
[ "$actual_tree" = "$expected_tree" ] || fail "patch apply tree $actual_tree does not match branch tree $expected_tree"
pass "patch applies to $base and reproduces branch tree in $patch_temp_repo"

printf 'Handoff artifacts verified for %s.\n' "$short_sha"
