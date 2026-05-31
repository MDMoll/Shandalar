#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

default_branch="$(git branch --show-current)"
branch="${default_branch:-codex/shandalar-crossover-updates}"
base="master"
dest=""
full_bundle=0
dry_run=0
skip_verify=0

usage() {
  cat <<'EOF'
Usage: tools/create-git-handoff-bundle.sh [options]

Creates a git bundle for handing off this branch when normal GitHub push
authentication is unavailable. By default, the bundle contains the current
branch's objects not already reachable from master, so the receiver should have
the repository's master branch.

Options:
  --branch REF     Branch/ref to bundle. Defaults to the current branch.
  --base REF       Base ref the receiver is expected to have. Defaults to master.
  --dest PATH      Bundle output path. Defaults to /private/tmp/<branch>-<sha>.bundle.
  --full           Include the base ref too, creating a larger but more standalone bundle.
  --dry-run        Validate refs and print the bundle command without writing a file.
  --skip-verify    Skip tools/verify-share-readiness.sh.
  -h, --help       Show this help.
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
    --full)
      full_bundle=1
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

git rev-parse --verify "$branch" >/dev/null || fail "unknown branch/ref: $branch"
git rev-parse --verify "$base" >/dev/null || fail "unknown base ref: $base"

short_sha="$(git rev-parse --short "$branch")"
safe_branch="$(printf '%s' "$branch" | tr '/: ' '---')"
dest="${dest:-/private/tmp/${safe_branch}-${short_sha}.bundle}"

case "$dest" in
  /*) ;;
  *) fail "destination must be an absolute path: $dest" ;;
esac

[ ! -e "$dest" ] || fail "destination already exists: $dest"

if [ "$skip_verify" != "1" ]; then
  tools/verify-share-readiness.sh
else
  printf 'warning: skipped tools/verify-share-readiness.sh\n' >&2
fi

if [ "$full_bundle" = "1" ]; then
  bundle_args=("$branch" "$base")
else
  bundle_args=("$branch" "^$base")
fi

printf 'Bundle destination: %s\n' "$dest"
printf 'Bundle branch: %s\n' "$branch"
if [ "$full_bundle" = "1" ]; then
  printf 'Bundle mode: full, includes base ref %s\n' "$base"
else
  printf 'Bundle mode: incremental, assumes receiver has %s\n' "$base"
fi

if [ "$dry_run" = "1" ]; then
  printf 'Dry run only; would run: git bundle create %q' "$dest"
  printf ' %q' "${bundle_args[@]}"
  printf '\n'
  exit 0
fi

git bundle create "$dest" "${bundle_args[@]}"
git bundle verify "$dest"
printf 'Created git handoff bundle: %s\n' "$dest"
