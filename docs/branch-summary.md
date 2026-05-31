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
| Share hygiene | Root `.gitattributes`, controlled-maintenance release scope, distribution caution, security-scan notes, docs index, local helper scope, and generated-evidence scope are now documented. | [share-readiness.md](share-readiness.md), [release-scope.md](release-scope.md), [distribution.md](distribution.md), [security-scan.md](security-scan.md), [README.md](README.md) |
| Automated checks | A verifier checks clean tree state, ignored local clutter, tracked ignored files, binary attributes, protected cleanup false positives, runtime-manifest hashes, representative patch bytes, tracked save/local-state inventory, security-scan target inventory, core docs, ASCII maintained text, docs index coverage, and local Markdown links. | [../tools/verify-share-readiness.sh](../tools/verify-share-readiness.sh) |

## Ready To Say

| Claim | Evidence |
| --- | --- |
| The branch is organized enough for a maintainer to navigate. | [README.md](README.md), [README](../README.md), and [share-readiness.md](share-readiness.md). |
| The high-confidence non-runtime clutter from the limited plan is archived, not deleted. | [reorganization.md](reorganization.md). |
| The major runtime files and patches are documented with repeatable checks. | [verified-on-this-machine.md](verified-on-this-machine.md) and `tools/verify-share-readiness.sh`. |
| Remaining cleanup candidates are intentionally deferred instead of silently removed. | [cleanup-audit.md](cleanup-audit.md), [cleanup-move-plan.md](cleanup-move-plan.md), [completion-audit.md](completion-audit.md), and [gaps.md](gaps.md). |

## Do Not Claim Yet

| Claim | Why not |
| --- | --- |
| Fully gameplay verified. | Manual visible Windows/CrossOver gameplay tests are still required. |
| Public redistribution approved. | No repo-level license was found and bundled rightsholder/trademark notices are present. |
| Patch/docs-only package prepared. | The current branch is scoped for controlled maintenance; no patch-only package has been prepared or tested. |
| Malware scanned. | No named malware scanner result is recorded. |
| Duplicate assets are safe to remove. | The full duplicate graph is measured, but no canonical runtime/package path has been chosen by launch-copy testing. |
| Save-state files are public fixtures. | Save/load behavior still needs a manual test before deciding whether to archive or keep them. |

## Before Pushing

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
tools/verify-share-readiness.sh
git log --oneline -10
git push -u origin codex/shandalar-crossover-updates
```

If the push fails with an HTTPS credential error, run the same push command
from an authenticated local terminal.
