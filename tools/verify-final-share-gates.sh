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

info() {
  printf 'info: %s\n' "$*"
}

[ -n "$branch" ] || fail "current checkout is detached; pass a branch name explicitly"

failed=0

record_failure() {
  printf 'FAIL: %s\n' "$*" >&2
  failed=1
}

tools/verify-share-readiness.sh
pass "automated local share-readiness checks passed"

if tools/verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md --show-missing; then
  pass "manual gameplay evidence is complete"
else
  record_failure "manual gameplay evidence is incomplete; fill docs/manual-gameplay-verification.md and rerun tools/verify-manual-gameplay-results.sh --doc docs/manual-gameplay-verification.md --show-missing"
fi

if tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all; then
  pass "security scan results cover all tracked scan targets"
else
  record_failure "security scan evidence is incomplete; create security-scan-results.tsv and rerun tools/verify-security-scan-results.sh --require-all"
fi

if [ "$failed" -eq 0 ]; then
  local_sha="$(git rev-parse "$branch")"
  if remote_ref="$(git ls-remote --heads origin "refs/heads/$branch" | awk '{print $1}')"; then
    if [ -z "$remote_ref" ]; then
      record_failure "origin does not report branch refs/heads/$branch"
    elif [ "$remote_ref" != "$local_sha" ]; then
      record_failure "origin/$branch is $remote_ref, expected $local_sha"
    else
      pass "origin/$branch matches local branch tip"
    fi
  else
    record_failure "could not query origin for refs/heads/$branch"
  fi
else
  info "origin branch query deferred until manual gameplay and security scan evidence gates pass"
fi

if [ "$failed" -ne 0 ]; then
  fail "final controlled-maintenance share gates failed for $branch at $(git rev-parse --short "$branch")"
fi

printf 'Final controlled-maintenance share gates passed for %s at %s.\n' "$branch" "$(git rev-parse --short "$branch")"
