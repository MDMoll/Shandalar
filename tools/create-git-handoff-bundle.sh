#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

default_branch="$(git branch --show-current)"
branch="${default_branch:-HEAD}"
base="master"
dest=""
full_bundle=0
dry_run=0
skip_verify=0
replace=0

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
  --replace        Overwrite only the exact destination and checksum sidecar if they exist as regular files.
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
    --replace)
      replace=1
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
dest_dir="$(dirname "$dest")"
dest_name="$(basename "$dest")"
checksum_dest="${dest}.sha256"
checksum_name="$(basename "$checksum_dest")"
if [[ "$branch" == refs/heads/* ]]; then
  source_ref="$branch"
  local_branch="${branch#refs/heads/}"
else
  source_ref="refs/heads/$branch"
  local_branch="$branch"
fi
dest_ref="refs/heads/$local_branch"

case "$dest" in
  /*) ;;
  *) fail "destination must be an absolute path: $dest" ;;
esac

check_output_path() {
  local label="$1"
  local path="$2"

  [ ! -L "$path" ] || fail "$label exists and is a symlink, refusing to replace: $path"
  if [ -e "$path" ] && [ ! -f "$path" ]; then
    fail "$label exists and is not a regular file: $path"
  fi
}

check_output_path "destination" "$dest"
check_output_path "checksum sidecar" "$checksum_dest"

if [ -e "$dest" ]; then
  if [ "$dry_run" = "1" ]; then
    if [ "$replace" = "1" ]; then
      printf 'warning: destination already exists; dry-run would replace it: %s\n' "$dest" >&2
    else
      printf 'warning: destination already exists, but dry-run will not overwrite it: %s\n' "$dest" >&2
    fi
  elif [ "$replace" = "1" ]; then
    printf 'Replacing existing destination: %s\n' "$dest" >&2
  else
    fail "destination already exists: $dest"
  fi
fi
if [ -e "$checksum_dest" ]; then
  if [ "$dry_run" = "1" ]; then
    if [ "$replace" = "1" ]; then
      printf 'warning: checksum sidecar already exists; dry-run would replace it: %s\n' "$checksum_dest" >&2
    else
      printf 'warning: checksum sidecar already exists, but dry-run will not overwrite it: %s\n' "$checksum_dest" >&2
    fi
  elif [ "$replace" = "1" ]; then
    printf 'Replacing existing checksum sidecar: %s\n' "$checksum_dest" >&2
  else
    fail "checksum sidecar already exists: $checksum_dest"
  fi
fi

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
printf 'Checksum sidecar: %s\n' "$checksum_dest"
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
  printf 'Dry run only; would write checksum from bundle directory: shasum -a 256 %q > %q\n' "$dest_name" "$checksum_name"
  printf 'Receiver checksum command from bundle directory: shasum -a 256 -c %q\n' "$checksum_name"
  printf 'Receiver verify command: git bundle verify %q\n' "$dest"
  printf 'Receiver fetch command: git fetch %q %q\n' "$dest" "$source_ref:$dest_ref"
  exit 0
fi

if [ "$replace" = "1" ]; then
  rm -f -- "$dest" "$checksum_dest"
fi

git bundle create "$dest" "${bundle_args[@]}"
git bundle verify "$dest"
(
  cd "$dest_dir"
  shasum -a 256 "$dest_name" > "$checksum_name"
)
printf 'Created git handoff bundle: %s\n' "$dest"
printf 'Created checksum sidecar: %s\n' "$checksum_dest"
printf 'Receiver checksum command from bundle directory: shasum -a 256 -c %q\n' "$checksum_name"
printf 'Receiver verify command: git bundle verify %q\n' "$dest"
printf 'Receiver fetch command: git fetch %q %q\n' "$dest" "$source_ref:$dest_ref"
