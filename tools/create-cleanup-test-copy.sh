#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
default_dest="/private/tmp/shandalar-cleanup-test-$(date +%Y%m%d-%H%M%S)"
dest=""
allow_dirty=0
allow_ignored_local=0
skip_verify=0

usage() {
  cat <<'EOF'
Usage: tools/create-cleanup-test-copy.sh [options] [destination]

Creates a disposable full-checkout copy for cleanup launch testing. The script
refuses to overwrite an existing path and runs tools/verify-share-readiness.sh
first unless --skip-verify is passed.

Options:
  --allow-dirty           Allow an in-progress working tree for copy experiments.
  --allow-ignored-local   Allow ignored local clutter such as .DS_Store.
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
  env "${env_args[@]}" tools/verify-share-readiness.sh
else
  printf 'warning: skipped tools/verify-share-readiness.sh\n' >&2
fi

mkdir -p "$(dirname "$dest")"

if command -v ditto >/dev/null 2>&1; then
  ditto "$repo_root" "$dest"
else
  cp -R "$repo_root" "$dest"
fi

printf 'Created cleanup test copy: %s\n' "$dest"
printf 'Next: test candidate moves only inside that copy, then record visible launch results in docs/cleanup-launch-copy-test.md.\n'
