#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

base="master"
branch="HEAD"
summary=0

usage() {
  cat <<'EOF'
Usage: tools/list-branch-delta.sh [options]

Lists tracked file changes on this branch relative to a base ref as TSV. This
is a planning aid for review, handoff, and future patch-only package work; it
does not create a patch or prove redistribution rights.

Options:
  --base REF    Base ref to compare against. Defaults to master.
  --branch REF  Branch/ref to inspect. Defaults to HEAD.
  --summary     Print a Markdown summary instead of TSV rows.
  -h, --help    Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

kind_for_path() {
  case "$1" in
    .gitattributes|.gitignore)
      printf 'repo-metadata'
      ;;
    README.md|readme.md|AGENTS.md|docs/*|archive/README.md|local/README.md|tools/README.md)
      printf 'documentation'
      ;;
    archive/*)
      printf 'archive-evidence'
      ;;
    *.7z|*.7Z|*.zip|*.ZIP|*.rar|*.RAR)
      printf 'compressed-archive'
      ;;
    CardArtNew/Thumbs.db)
      printf 'generated-local'
      ;;
    tools/*.sh|local/*.sh|local/*/*.sh)
      printf 'shell-tool'
      ;;
    tools/*.py|local/*.py|local/*/*.py)
      printf 'python-tool'
      ;;
    src/*|Program/src/*)
      printf 'source'
      ;;
    *.exe|*.EXE)
      printf 'pe-executable'
      ;;
    *.dll|*.DLL)
      printf 'pe-dll'
      ;;
    Program/FaceArt/*|*.pcx|*.PCX|*.pic|*.PIC|*.spr|*.SPR|*.res|*.RES)
      printf 'art-resource'
      ;;
    *.dat|*.DAT|*.csv|*.CSV|*.txt|*.TXT|*.ini|*.INI)
      printf 'data-or-config'
      ;;
    *.bat|*.BAT|*.cmd|*.CMD|*.vbs|*.VBS|*.pl|*.PL)
      printf 'script'
      ;;
    *)
      printf 'other'
      ;;
  esac
}

blob_bytes() {
  git cat-file -s "$branch:$1"
}

blob_sha256() {
  git cat-file blob "$branch:$1" | shasum -a 256 | awk '{print $1}'
}

emit_tsv() {
  printf 'status\tpath\told_path\tkind\tbytes\tsha256\n'

  git diff --name-status -M "$base...$branch" |
  while IFS=$'\t' read -r status path_a path_b; do
    [ -n "${status:-}" ] || continue

    old_path="-"
    path="$path_a"

    case "$status" in
      R*|C*)
        old_path="$path_a"
        path="$path_b"
        ;;
    esac

    kind="$(kind_for_path "$path")"

    if [ "$status" = "D" ]; then
      bytes="-"
      sha256="-"
    else
      bytes="$(blob_bytes "$path")"
      sha256="$(blob_sha256 "$path")"
    fi

    printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$status" "$path" "$old_path" "$kind" "$bytes" "$sha256"
  done
}

emit_summary() {
  delta_tsv="$(emit_tsv)"
  total_rows="$(printf '%s\n' "$delta_tsv" | awk 'NR > 1 {count++} END {print count+0}')"
  merge_base_sha="$(git merge-base "$base" "$branch")"

  printf '# Branch Delta Summary\n\n'
  printf '| Field | Value |\n'
  printf '| --- | --- |\n'
  printf '| Base | `%s` (`%s`) |\n' "$base" "$(git rev-parse --short "$base")"
  printf '| Branch | `%s` (`%s`) |\n' "$branch" "$(git rev-parse --short "$branch")"
  printf '| Merge base | `%s` |\n' "$(git rev-parse --short "$merge_base_sha")"
  printf '| Changed paths | %s |\n' "$total_rows"

  printf '\n## By Kind\n\n'
  printf '| Kind | Files | Bytes |\n'
  printf '| --- | ---: | ---: |\n'
  printf '%s\n' "$delta_tsv" |
    awk -F '\t' 'NR > 1 {count[$4]++; if ($5 != "-") bytes[$4] += $5} END {for (k in count) print k "\t" count[k] "\t" bytes[k]+0}' |
    sort |
    awk -F '\t' '{printf "| `%s` | %d | %d |\n", $1, $2, $3}'

  printf '\n## By Status\n\n'
  printf '| Status | Files |\n'
  printf '| --- | ---: |\n'
  printf '%s\n' "$delta_tsv" |
    awk -F '\t' 'NR > 1 {count[$1]++} END {for (s in count) print s "\t" count[s]}' |
    sort |
    awk -F '\t' '{printf "| `%s` | %d |\n", $1, $2}'
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
    --summary)
      summary=1
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

git rev-parse --verify "$base" >/dev/null || fail "unknown base ref: $base"
git rev-parse --verify "$branch" >/dev/null || fail "unknown branch/ref: $branch"
git merge-base "$base" "$branch" >/dev/null || fail "no merge base between $base and $branch"

if [ "$summary" = "1" ]; then
  emit_summary
else
  emit_tsv
fi
