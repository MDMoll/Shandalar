#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

branch="$(git branch --show-current)"
commit_short="$(git rev-parse --short HEAD)"
commit_full="$(git rev-parse HEAD)"
remote="$(git remote get-url origin 2>/dev/null || printf 'not configured')"
upstream="$(git rev-parse --abbrev-ref --symbolic-full-name '@{u}' 2>/dev/null || true)"
status="$(git status --short --untracked-files=all)"
status_value="clean"
if [ -n "$status" ]; then
  status_lines="$(printf '%s\n' "$status" | sed '/^$/d' | wc -l | tr -d ' ')"
  status_value="dirty (${status_lines} status entries)"
fi

ignored_local="$(git status --ignored --short --untracked-files=all | awk '/^!! / {print substr($0, 4)}' | awk '$0 !~ /^\.codex\// && $0 != "security-scan-results.tsv" {count++} END {print count+0}')"
scan_evidence_status="absent"
if [ -f security-scan-results.tsv ]; then
  scan_evidence_status="present as ignored local evidence"
fi
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
scan_results_path="security-scan-results.tsv"
manual_gameplay_doc="docs/manual-gameplay-verification.md"
security_inventory_note='`tools/list-security-scan-targets.sh`; scanner still needs to be run separately'
if [ -f "$scan_results_path" ] && tools/verify-security-scan-results.sh --results "$scan_results_path" --require-all >/dev/null 2>&1; then
  security_inventory_note='`tools/list-security-scan-targets.sh`; local scanner results validate for all current targets'
fi

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

security_scan_results_status() {
  if [ ! -f "$scan_results_path" ]; then
    printf 'missing; no scanner results recorded'
    return
  fi

  if tools/verify-security-scan-results.sh --results "$scan_results_path" >/dev/null 2>&1; then
    if tools/verify-security-scan-results.sh --results "$scan_results_path" --require-all >/dev/null 2>&1; then
      printf 'present; all %s tracked target rows validate' "$security_target_count"
    else
      valid_rows="$(awk -F '\t' 'NR > 1 && NF > 0 {count++} END {print count+0}' "$scan_results_path")"
      printf 'present; %s recorded row(s) validate, but full coverage is incomplete' "$valid_rows"
    fi
  else
    printf 'present; validation FAILED'
  fi
}

security_gate_status() {
  if [ -f "$scan_results_path" ] && tools/verify-security-scan-results.sh --results "$scan_results_path" --require-all >/dev/null 2>&1; then
    printf 'Satisfied locally; named scanner rows validate for all current tracked scan targets. See `docs/security-scan.md`.'
  else
    printf 'Needs a named scanner/version/result in `docs/security-scan.md`; validate local TSV evidence with `tools/verify-security-scan-results.sh --require-all`.'
  fi
}

strict_final_status() {
  if [ -f "$scan_results_path" ] && tools/verify-security-scan-results.sh --results "$scan_results_path" --require-all >/dev/null 2>&1; then
    printf '`tools/verify-final-share-gates.sh` should still fail until manual gameplay evidence is complete.'
  else
    printf '`tools/verify-final-share-gates.sh` should still fail until manual gameplay and security scan evidence are complete.'
  fi
}

manual_gameplay_status() {
  tools/verify-manual-gameplay-results.sh --doc "$manual_gameplay_doc" --allow-incomplete | sed 's/^ok: //'
}

push_gate_status() {
  if [ -z "$upstream" ]; then
    printf 'Needs authenticated `git push -u origin %s`; no upstream branch is configured. See `docs/push-auth.md`.' "$branch"
    return
  fi

  upstream_sha="$(git rev-parse "$upstream" 2>/dev/null || true)"
  if [ "$upstream_sha" = "$commit_full" ]; then
    printf 'Pushed; local `HEAD` matches `%s` at `%s`.' "$upstream" "$commit_short"
  else
    printf 'Needs push or pull review; local `HEAD` is `%s`, but `%s` is `%s`.' "$commit_short" "$upstream" "$(git rev-parse --short "$upstream" 2>/dev/null || printf 'unknown')"
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
printf '| Ignored local scan evidence | %s |\n' "$scan_evidence_status"
printf '| Tracked ignored files | `%s` |\n' "$tracked_ignored"
printf '| Remote | `%s` |\n' "$remote"
printf '| Upstream | `%s` |\n' "${upstream:-none}"

printf '\n## Inventory Snapshot\n\n'
printf '| Inventory | Current count | Notes |\n'
printf '| --- | ---: | --- |\n'
printf '| Branch delta rows | %s | `tools/list-branch-delta.sh`; rows classified as `other`: %s |\n' "$branch_delta_count" "$branch_delta_other_count"
printf '| Security scan targets | %s | %s |\n' "$security_target_count" "$security_inventory_note"

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

printf '\n## Security Scan Results\n\n'
printf '| Field | Value |\n'
printf '| --- | --- |\n'
printf '| Results file | `%s` |\n' "$scan_results_path"
printf '| Validation | %s |\n' "$(security_scan_results_status)"
printf '| Full-coverage command | `tools/verify-security-scan-results.sh --results %s --require-all` |\n' "$scan_results_path"

printf '\n## Manual Gameplay Results\n\n'
printf '| Field | Value |\n'
printf '| --- | --- |\n'
printf '| Results doc | `%s` |\n' "$manual_gameplay_doc"
printf '| Validation | %s |\n' "$(manual_gameplay_status)"
printf '| Completion command | `tools/verify-manual-gameplay-results.sh --doc %s` |\n' "$manual_gameplay_doc"
printf '| Gap command | `tools/verify-manual-gameplay-results.sh --doc %s --allow-incomplete --show-missing` |\n' "$manual_gameplay_doc"

printf '\n## Final Gates\n\n'
printf '| Gate | Current status |\n'
printf '| --- | --- |\n'
printf '| GitHub push | %s |\n' "$(push_gate_status)"
printf '| Manual gameplay | Needs visible pass/fail evidence in `docs/manual-gameplay-verification.md`; validate it with `tools/verify-manual-gameplay-results.sh`. |\n'
printf '| Security scan | %s |\n' "$(security_gate_status)"
printf '| Public distribution | Not approved by current evidence; see `docs/release-scope.md` and `docs/distribution.md`. |\n'
printf '| Additional cleanup moves | Deferred unless explicitly approved and launch-copy tested. |\n'
printf '| Strict final verifier | %s |\n' "$(strict_final_status)"
