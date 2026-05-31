#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

branch="${1:-$(git branch --show-current)}"

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

[ -n "$branch" ] || fail "current checkout is detached; pass a branch name explicitly"

tools/verify-share-readiness.sh
pass "automated local share-readiness checks passed"

if tools/verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md; then
  pass "manual gameplay evidence is complete"
else
  fail "manual gameplay evidence is incomplete; fill docs/manual-gameplay-verification.md and rerun tools/verify-manual-gameplay-results.sh"
fi

if tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all; then
  pass "security scan results cover all tracked scan targets"
else
  fail "security scan evidence is incomplete; create security-scan-results.tsv and rerun tools/verify-security-scan-results.sh --require-all"
fi

local_sha="$(git rev-parse "$branch")"
remote_ref="$(git ls-remote --heads origin "refs/heads/$branch" | awk '{print $1}')"
[ -n "$remote_ref" ] || fail "origin does not report branch refs/heads/$branch"
[ "$remote_ref" = "$local_sha" ] || fail "origin/$branch is $remote_ref, expected $local_sha"
pass "origin/$branch matches local branch tip"

printf 'Final controlled-maintenance share gates passed for %s at %s.\n' "$branch" "$(git rev-parse --short "$branch")"
