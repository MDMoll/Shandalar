# Branch Summary

Branch: `codex/shandalar-crossover-updates`

This branch turns the checkout into a more navigable maintenance tree while
preserving the bundled game/runtime layout. It deletes nothing and avoids
moving runtime-like assets without explicit approval and launch-copy testing.

## What Changed

| Area | Summary | Pointers |
| --- | --- | --- |
| Runtime patches | Active `Shandalar.exe`, `FaceMaker.exe`, `Magic.exe`, and `ManalinkEh.dll` were patched and documented with hashes and representative bytes. | [runtime-manifest.md](runtime-manifest.md), [verified-on-this-machine.md](verified-on-this-machine.md), [running.md](running.md), [magic-exe.md](magic-exe.md), [bugs/](bugs/) |
| Limited archive reorg | Obvious generated, debug, local-helper, backup, historical doc, and historical link files were moved under `archive/` with history preserved. | [reorganization.md](reorganization.md), [../archive/README.md](../archive/README.md) |
| Cleanup evidence | Save-state files, generated caches, duplicate assets, stale references, and risky cleanup candidates are mapped with evidence and confidence. | [cleanup-audit.md](cleanup-audit.md), [cleanup-move-plan.md](cleanup-move-plan.md), [duplicate-audit.md](duplicate-audit.md), [save-state.md](save-state.md) |
| Share hygiene | Root `.gitattributes`, controlled-maintenance release scope, patch-only planning, distribution caution, security-scan notes, docs index, local helper scope, and generated-evidence scope are now documented. | [share-readiness.md](share-readiness.md), [release-scope.md](release-scope.md), [patch-package-plan.md](patch-package-plan.md), [distribution.md](distribution.md), [security-scan.md](security-scan.md), [README.md](README.md) |
| Automated checks | Verifiers check clean tree state, ignored local clutter, generated report/handoff ignore rules, tracked ignored files, binary attributes, protected cleanup false positives, runtime-manifest hashes, representative patch bytes, tracked save/local-state inventory, security-scan target inventory, branch-delta inventory shape, core docs, current CrossOver quick-start guidance, ASCII maintained text, docs index coverage, local Markdown links, and final controlled-maintenance gates. | [../tools/verify-share-readiness.sh](../tools/verify-share-readiness.sh), [../tools/verify-final-share-gates.sh](../tools/verify-final-share-gates.sh) |
| Handoff helpers | Read-only helpers print current share status, verify that report against live counts/statuses, print gameplay/security evidence baselines, dry-run cleanup copies, create a Git bundle plus checksum fallback, and create temporary binary patch artifacts for patch-only planning. | [../tools/README.md](../tools/README.md), [share-readiness.md](share-readiness.md), [manual-gameplay-verification.md](manual-gameplay-verification.md), [security-scan.md](security-scan.md), [git-handoff.md](git-handoff.md), [patch-package-plan.md](patch-package-plan.md) |

## Ready To Say

| Claim | Evidence |
| --- | --- |
| The branch is organized enough for a maintainer to navigate. | [README.md](README.md), [README](../README.md), and [share-readiness.md](share-readiness.md). |
| The high-confidence non-runtime clutter from the limited plan is archived, not deleted. | [reorganization.md](reorganization.md). |
| The major runtime files and patches are documented with repeatable checks. | [verified-on-this-machine.md](verified-on-this-machine.md) and `tools/verify-share-readiness.sh`. |
| Remaining cleanup candidates are intentionally deferred instead of silently removed. | [cleanup-audit.md](cleanup-audit.md), [cleanup-move-plan.md](cleanup-move-plan.md), [completion-audit.md](completion-audit.md), and [gaps.md](gaps.md). |
| The branch push handoff is explicit, with owner-side auth paths, bundle/patch artifacts, checksum sidecars, and verifier coverage for credential-blocked environments. | [git-handoff.md](git-handoff.md), [push-auth.md](push-auth.md), `tools/create-git-handoff-bundle.sh`, `tools/create-patch-package.sh`, and `tools/verify-handoff-artifacts.sh`. |

## Do Not Claim Yet

| Claim | Why not |
| --- | --- |
| Fully gameplay verified. | Manual visible Windows/CrossOver gameplay tests are still required. |
| Final controlled-maintenance gates passed. | `tools/verify-final-share-gates.sh` should fail until push, manual gameplay, and security scan evidence are complete. |
| Public redistribution approved. | No repo-level license was found and bundled rightsholder/trademark notices are present. |
| Patch/docs-only package prepared. | The current branch is scoped for controlled maintenance; [patch-package-plan.md](patch-package-plan.md) records a branch-delta inventory path, but no patch artifact has been prepared or tested. |
| Malware scanned. | No named malware scanner result is recorded. |
| Duplicate assets are safe to remove. | The full duplicate graph is measured, but no canonical runtime/package path has been chosen by launch-copy testing. |
| Save-state files are public fixtures. | Save/load behavior still needs a manual test before deciding whether to archive or keep them. |

## Before Pushing

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
tools/print-share-status.sh
tools/verify-share-readiness.sh
tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts
tools/print-manual-gameplay-baseline.sh
tools/print-security-scan-baseline.sh
git log --oneline -10
git push -u origin codex/shandalar-crossover-updates
```

If the push fails with an HTTPS credential error, run the same push command
from an authenticated local terminal. See [push-auth.md](push-auth.md) for
HTTPS token, GitHub CLI, and SSH options. If credentials are still unavailable,
use `tools/create-git-handoff-bundle.sh --replace` to recreate a Git-history
handoff bundle for the current commit.
The helpers now print receiver-side checksum, `git bundle verify`, `git fetch`,
and patch-apply commands; `tools/verify-handoff-artifacts.sh` checks the
default artifacts for the current commit. See [git-handoff.md](git-handoff.md).
