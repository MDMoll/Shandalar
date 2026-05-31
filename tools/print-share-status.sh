#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

branch="$(git branch --show-current)"
commit_short="$(git rev-parse --short HEAD)"
commit_full="$(git rev-parse HEAD)"
remote="$(git remote get-url origin 2>/dev/null || printf 'not configured')"
status="$(git status --short --untracked-files=all)"
status_value="clean"
if [ -n "$status" ]; then
  status_lines="$(printf '%s\n' "$status" | sed '/^$/d' | wc -l | tr -d ' ')"
  status_value="dirty (${status_lines} status entries)"
fi

ignored_local="$(git status --ignored --short --untracked-files=all | awk '/^!! / {count++} END {print count+0}')"
tracked_ignored="$(git ls-files -ci --exclude-standard | tr '\n' ' ')"
tracked_ignored="${tracked_ignored% }"
[ -n "$tracked_ignored" ] || tracked_ignored="none"

security_target_count="$(tools/list-security-scan-targets.sh | awk 'NR > 1 {count++} END {print count+0}')"
branch_delta="$(tools/list-branch-delta.sh)"
branch_delta_count="$(printf '%s\n' "$branch_delta" | awk 'NR > 1 {count++} END {print count+0}')"
branch_delta_other_count="$(printf '%s\n' "$branch_delta" | awk -F '\t' 'NR > 1 && $4 == "other" {count++} END {print count+0}')"
safe_branch="$(printf '%s' "$branch" | tr '/: ' '---')"
bundle_path="/private/tmp/${safe_branch}-${commit_short}.bundle"
bundle_checksum_path="${bundle_path}.sha256"
patch_path="/private/tmp/${safe_branch}-${commit_short}.patch"
patch_checksum_path="${patch_path}.sha256"

artifact_status() {
  local artifact="$1"
  local checksum="$2"

  if [ ! -f "$artifact" ]; then
    printf 'missing'
    return
  fi

  if [ ! -f "$checksum" ]; then
    printf 'present, missing checksum sidecar'
    return
  fi

  if (
    cd "$(dirname "$artifact")"
    shasum -a 256 -c "$(basename "$checksum")" >/dev/null 2>&1
  ); then
    printf 'present, checksum OK'
  else
    printf 'present, checksum FAILED'
  fi
}

artifact_sha() {
  local artifact="$1"

  if [ -f "$artifact" ]; then
    shasum -a 256 "$artifact" | awk '{print $1}'
  else
    printf 'missing'
  fi
}

printf '# Share Status\n\n'
printf 'This is a current-state report for handoff. It does not push to GitHub, run a malware scanner, or prove gameplay.\n\n'

printf '## Current Branch\n\n'
printf '| Field | Value |\n'
printf '| --- | --- |\n'
printf '| Repo path | `%s` |\n' "$repo_root"
printf '| Branch | `%s` |\n' "$branch"
printf '| Commit | `%s` (`%s`) |\n' "$commit_short" "$commit_full"
printf '| Git status | %s |\n' "$status_value"
printf '| Ignored local files | %s |\n' "$ignored_local"
printf '| Tracked ignored files | `%s` |\n' "$tracked_ignored"
printf '| Remote | `%s` |\n' "$remote"

printf '\n## Inventory Snapshot\n\n'
printf '| Inventory | Current count | Notes |\n'
printf '| --- | ---: | --- |\n'
printf '| Branch delta rows | %s | `tools/list-branch-delta.sh`; rows classified as `other`: %s |\n' "$branch_delta_count" "$branch_delta_other_count"
printf '| Security scan targets | %s | `tools/list-security-scan-targets.sh`; scanner still needs to be run separately |\n' "$security_target_count"

printf '\n## Handoff Commands\n\n'
printf '```sh\n'
printf 'git status --short --untracked-files=all\n'
printf 'tools/verify-share-readiness.sh\n'
printf 'tools/create-git-handoff-bundle.sh --replace\n'
printf 'tools/create-patch-package.sh --replace --verify-apply\n'
printf 'tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts\n'
printf 'git push -u origin %q\n' "$branch"
printf '```\n'

printf '\n## Default Handoff Artifact Paths\n\n'
printf '| Artifact | Path | Status |\n'
printf '| --- | --- | --- |\n'
printf '| Git bundle | `%s` | %s |\n' "$bundle_path" "$(artifact_status "$bundle_path" "$bundle_checksum_path")"
printf '| Git bundle checksum | `%s` | %s |\n' "$bundle_checksum_path" "$([ -f "$bundle_checksum_path" ] && printf 'present' || printf 'missing')"
printf '| Binary patch | `%s` | %s |\n' "$patch_path" "$(artifact_status "$patch_path" "$patch_checksum_path")"
printf '| Binary patch checksum | `%s` | %s |\n' "$patch_checksum_path" "$([ -f "$patch_checksum_path" ] && printf 'present' || printf 'missing')"

printf '\n## Default Handoff Artifact Hashes\n\n'
printf '| Artifact | SHA-256 |\n'
printf '| --- | --- |\n'
printf '| Git bundle | `%s` |\n' "$(artifact_sha "$bundle_path")"
printf '| Binary patch | `%s` |\n' "$(artifact_sha "$patch_path")"

printf '\n## Final Gates\n\n'
printf '| Gate | Current status |\n'
printf '| --- | --- |\n'
printf '| GitHub push | Needs authenticated `git push -u origin %s` from an environment that can answer GitHub credentials; see `docs/push-auth.md`. |\n' "$branch"
printf '| Manual gameplay | Needs visible pass/fail evidence in `docs/manual-gameplay-verification.md`. |\n'
printf '| Security scan | Needs a named scanner/version/result in `docs/security-scan.md`. |\n'
printf '| Public distribution | Not approved by current evidence; see `docs/release-scope.md` and `docs/distribution.md`. |\n'
printf '| Additional cleanup moves | Deferred unless explicitly approved and launch-copy tested. |\n'
