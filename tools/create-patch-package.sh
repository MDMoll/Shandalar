#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

default_branch="$(git branch --show-current)"
branch="${default_branch:-HEAD}"
base="master"
dest=""
dry_run=0
skip_verify=0
verify_apply=0
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
Usage: tools/create-patch-package.sh [options]

Creates a binary git patch plus .sha256 sidecar for the current branch relative
to a base ref. This is a technical patch/docs-only planning artifact, not proof
of redistribution rights or gameplay readiness.

Options:
  --branch REF       Branch/ref to package. Defaults to the current branch.
  --base REF         Base ref to diff against. Defaults to master.
  --dest PATH        Patch output path. Defaults to /private/tmp/<branch>-<sha>.patch.
  --verify-apply     Apply the patch in a disposable clone and compare the tree.
  --dry-run          Validate refs and print commands without writing a patch.
  --skip-verify      Skip tools/verify-share-readiness.sh.
  -h, --help         Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --branch)
      shift
      [ "$#" -gt 0 ] || fail "--branch requires a value"
      branch="$1"
      ;;
    --base)
      shift
      [ "$#" -gt 0 ] || fail "--base requires a value"
      base="$1"
      ;;
    --dest)
      shift
      [ "$#" -gt 0 ] || fail "--dest requires a value"
      dest="$1"
      ;;
    --verify-apply)
      verify_apply=1
      ;;
    --dry-run)
      dry_run=1
      ;;
    --skip-verify)
      skip_verify=1
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

git rev-parse --verify "$branch^{commit}" >/dev/null || fail "unknown branch/ref: $branch"
git rev-parse --verify "$base^{commit}" >/dev/null || fail "unknown base ref: $base"
git merge-base "$base" "$branch" >/dev/null || fail "no merge base between $base and $branch"
base_commit="$(git rev-parse "$base^{commit}")"

short_sha="$(git rev-parse --short "$branch")"
safe_branch="$(printf '%s' "$branch" | tr '/: ' '---')"
dest="${dest:-/private/tmp/${safe_branch}-${short_sha}.patch}"
dest_dir="$(dirname "$dest")"
dest_name="$(basename "$dest")"
checksum_dest="${dest}.sha256"
checksum_name="$(basename "$checksum_dest")"

case "$dest" in
  /*) ;;
  *) fail "destination must be an absolute path: $dest" ;;
esac

if [ -e "$dest" ]; then
  if [ "$dry_run" = "1" ]; then
    printf 'warning: destination already exists, but dry-run will not overwrite it: %s\n' "$dest" >&2
  else
    fail "destination already exists: $dest"
  fi
fi
if [ -e "$checksum_dest" ]; then
  if [ "$dry_run" = "1" ]; then
    printf 'warning: checksum sidecar already exists, but dry-run will not overwrite it: %s\n' "$checksum_dest" >&2
  else
    fail "checksum sidecar already exists: $checksum_dest"
  fi
fi

if [ "$skip_verify" != "1" ]; then
  tools/verify-share-readiness.sh
else
  printf 'warning: skipped tools/verify-share-readiness.sh\n' >&2
fi

printf 'Patch destination: %s\n' "$dest"
printf 'Checksum sidecar: %s\n' "$checksum_dest"
printf 'Patch branch: %s\n' "$branch"
printf 'Patch base: %s\n' "$base"
printf 'Patch mode: binary git diff for %s...%s\n' "$base" "$branch"

if [ "$dry_run" = "1" ]; then
  printf 'Dry run only; would run: git diff --binary %q...%q > %q\n' "$base" "$branch" "$dest"
  printf 'Dry run only; would write checksum from patch directory: shasum -a 256 %q > %q\n' "$dest_name" "$checksum_name"
  if [ "$verify_apply" = "1" ]; then
    printf 'Dry run only; would apply patch in a disposable clone and compare tree to %q.\n' "$branch"
  fi
  printf 'Receiver checksum command from patch directory: shasum -a 256 -c %q\n' "$checksum_name"
  printf 'Receiver check command from base index: git apply --check --cached --binary --whitespace=nowarn %q\n' "$dest"
  printf 'Receiver apply command from base index: git apply --cached --binary --whitespace=nowarn %q\n' "$dest"
  exit 0
fi

git diff --binary "$base...$branch" > "$dest"
(
  cd "$dest_dir"
  shasum -a 256 "$dest_name" > "$checksum_name"
)

printf 'Created patch package: %s\n' "$dest"
printf 'Created checksum sidecar: %s\n' "$checksum_dest"

if [ "$verify_apply" = "1" ]; then
  temp_root="$(mktemp -d "/private/tmp/shandalar-patch-apply-${short_sha}.XXXXXX")"
  temp_paths+=("$temp_root")
  temp_repo="$temp_root/repo"
  expected_tree="$(git rev-parse "$branch^{tree}")"

  git clone --no-checkout "$repo_root" "$temp_repo" >/dev/null
  git -C "$temp_repo" read-tree "$base_commit"
  git -C "$temp_repo" apply --cached --binary --whitespace=nowarn "$dest"
  actual_tree="$(git -C "$temp_repo" write-tree)"
  [ "$actual_tree" = "$expected_tree" ] || fail "patch apply tree $actual_tree does not match $branch tree $expected_tree"
  printf 'Patch apply verified in %s; resulting tree matches %s.\n' "$temp_repo" "$branch"
fi

printf 'Receiver checksum command from patch directory: shasum -a 256 -c %q\n' "$checksum_name"
printf 'Receiver check command from base index: git apply --check --cached --binary --whitespace=nowarn %q\n' "$dest"
printf 'Receiver apply command from base index: git apply --cached --binary --whitespace=nowarn %q\n' "$dest"
