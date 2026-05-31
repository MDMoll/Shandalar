#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
default_dest="/private/tmp/shandalar-cleanup-test-$(date +%Y%m%d-%H%M%S)"
dest=""
allow_dirty=0
allow_ignored_local=0
skip_verify=0
dry_run=0
include_git=0

usage() {
  cat <<'EOF'
Usage: tools/create-cleanup-test-copy.sh [options] [destination]

Creates a disposable working-tree copy for cleanup launch testing. The script
omits .git by default, refuses to overwrite an existing path, and runs
tools/verify-share-readiness.sh first unless --skip-verify is passed.

Options:
  --allow-dirty           Allow an in-progress working tree for copy experiments.
  --allow-ignored-local   Allow ignored local clutter such as .DS_Store.
  --dry-run               Validate inputs and verifier state without copying.
  --include-git           Include .git in the copy; not needed for launch tests.
  --skip-verify           Skip tools/verify-share-readiness.sh.
  -h, --help              Show this help.

Default destination:
  /private/tmp/shandalar-cleanup-test-YYYYMMDD-HHMMSS
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --allow-dirty)
      allow_dirty=1
      ;;
    --allow-ignored-local)
      allow_ignored_local=1
      ;;
    --dry-run)
      dry_run=1
      ;;
    --include-git)
      include_git=1
      ;;
    --skip-verify)
      skip_verify=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      fail "unknown option: $1"
      ;;
    *)
      [ -z "$dest" ] || fail "only one destination may be provided"
      dest="$1"
      ;;
  esac
  shift
done

if [ "$#" -gt 0 ]; then
  [ -z "$dest" ] || fail "only one destination may be provided"
  dest="$1"
  shift
fi
[ "$#" -eq 0 ] || fail "unexpected extra arguments"

dest="${dest:-$default_dest}"

case "$dest" in
  /*) ;;
  *) fail "destination must be an absolute path: $dest" ;;
esac

[ ! -e "$dest" ] || fail "destination already exists: $dest"

cd "$repo_root"

if [ "$skip_verify" != "1" ]; then
  env_args=()
  [ "$allow_dirty" = "1" ] && env_args+=(ALLOW_DIRTY=1)
  [ "$allow_ignored_local" = "1" ] && env_args+=(ALLOW_IGNORED_LOCAL=1)
  if [ "${#env_args[@]}" -gt 0 ]; then
    env "${env_args[@]}" tools/verify-share-readiness.sh
  else
    tools/verify-share-readiness.sh
  fi
else
  printf 'warning: skipped tools/verify-share-readiness.sh\n' >&2
fi

if [ "$dry_run" = "1" ]; then
  if [ "$include_git" = "1" ]; then
    printf 'Dry run only; would create cleanup test copy including .git: %s\n' "$dest"
  else
    printf 'Dry run only; would create cleanup test copy without .git: %s\n' "$dest"
  fi
  exit 0
fi

mkdir -p "$(dirname "$dest")"

if [ "$include_git" = "1" ]; then
  if command -v ditto >/dev/null 2>&1; then
    ditto "$repo_root" "$dest"
  else
    cp -R "$repo_root" "$dest"
  fi
elif command -v rsync >/dev/null 2>&1; then
  mkdir -p "$dest"
  rsync -a --exclude '/.git/' "$repo_root"/ "$dest"/
elif tar --help >/dev/null 2>&1; then
  mkdir -p "$dest"
  (
    cd "$repo_root"
    tar --exclude './.git' -cf - .
  ) | (
    cd "$dest"
    tar -xf -
  )
elif command -v ditto >/dev/null 2>&1; then
  printf 'warning: rsync/tar unavailable; falling back to ditto including .git\n' >&2
  ditto "$repo_root" "$dest"
else
  printf 'warning: rsync/tar unavailable; falling back to cp including .git\n' >&2
  cp -R "$repo_root" "$dest"
fi

printf 'Created cleanup test copy: %s\n' "$dest"
printf 'Next: test candidate moves only inside that copy, then record visible launch results in docs/cleanup-launch-copy-test.md.\n'
