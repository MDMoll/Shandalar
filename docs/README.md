# Documentation Index

Start with the root [README](../README.md) for launch basics. Use this index
when you need the deeper evidence, cleanup notes, or test checklists.

## Start Here

| Doc | Use it for |
| --- | --- |
| [branch-summary.md](branch-summary.md) | Handoff summary for what this branch changes and what remains unproven. |
| [share-readiness.md](share-readiness.md) | Current branch checklist, push checklist, and remaining unproven claims. |
| [completion-audit.md](completion-audit.md) | Requirement-by-requirement status for the cleanup/share goal. |
| [git-handoff.md](git-handoff.md) | Exact branch, remote, pre-push checks, and push-auth status. |
| [push-auth.md](push-auth.md) | Owner-side HTTPS token, GitHub CLI, and SSH options for pushing the branch. |
| [release-scope.md](release-scope.md) | Current branch sharing scope and public-release boundaries. |
| [patch-package-plan.md](patch-package-plan.md) | Branch-delta inventory and restoration-test plan for any future patch/docs-only package. |
| [verified-on-this-machine.md](verified-on-this-machine.md) | Detailed local command/result evidence. |
| [running.md](running.md) | Windows, Wine, and CrossOver launch commands and test matrix. |
| [manual-gameplay-verification.md](manual-gameplay-verification.md) | Manual test plan and validator required before claiming the game works end to end. |
| [runtime-testing-policy.md](runtime-testing-policy.md) | Bounded runtime-testing rules, GUI automation stop conditions, and evidence fields. |
| [runtime-test-notes.md](runtime-test-notes.md) | Short chronological notes from bounded runtime attempts. |
| [crossover-macos.md](crossover-macos.md) | CrossOver bottle setup, app-defaults, desktop sizing, and troubleshooting. |
| [troubleshooting.md](troubleshooting.md) | Practical issue notes and current mitigations. |

## Runtime Details

| Doc | Use it for |
| --- | --- |
| [runtime-dependencies.md](runtime-dependencies.md) | PE import evidence and runtime dependency guidance. |
| [runtime-manifest.md](runtime-manifest.md) | SHA-256 manifest for active patched files and protected references. |
| [magic-exe.md](magic-exe.md) | Dedicated `Magic.exe` behavior, imports, and tests. |
| [magic-vs-shandalar-runtime.md](magic-vs-shandalar-runtime.md) | How Shandalar and Magic launch paths relate. |
| [command-line.md](command-line.md) | `--help`, `--e`, `--p`, and command-line evidence. |
| [building.md](building.md) | Source/build observations and blockers. |
| [architecture.md](architecture.md) | High-level repo organization. |

## Bug Notes

| Doc | Use it for |
| --- | --- |
| [bugs/create-dibsection-after-color.md](bugs/create-dibsection-after-color.md) | Start-color `CreateDIBSection` assertion and patch evidence. |
| [bugs/duel-freeze-damage-prevention.md](bugs/duel-freeze-damage-prevention.md) | Femeref/Samite/Kithkin damage-prevention freeze investigation. |
| [bugs/declared-attacker-undo.md](bugs/declared-attacker-undo.md) | Declared-attacker undo patch notes. |
| [adventure-map-movement.md](adventure-map-movement.md) | Same-arrow adventure-map stop patch notes. |

## Cleanup And Sharing

| Doc | Use it for |
| --- | --- |
| [file-inventory.md](file-inventory.md) | File counts, large directories, generated/local-state observations. |
| [install-roots.md](install-roots.md) | Install-root inventory, supported-layout decisions, and local-folder evidence. |
| [package-layout-cleanup.md](package-layout-cleanup.md) | Install-root cleanup decision log and exact duplicate archive removals. |
| [cleanup-audit.md](cleanup-audit.md) | Cleanup candidates with evidence and confidence. |
| [cleanup-removed-files.md](cleanup-removed-files.md) | Exact files removed by the safe cleanup pass, with evidence and rollback notes. |
| [repo-cleanup-plan.md](repo-cleanup-plan.md) | Cleanup decision log, remaining duplicate families, and next cleanup batches. |
| [cleanup-move-plan.md](cleanup-move-plan.md) | Pending cleanup moves that need approval or launch-copy testing. |
| [cleanup-launch-copy-test.md](cleanup-launch-copy-test.md) | Disposable-copy test plan before moving risky cleanup candidates. |
| [duplicate-audit.md](duplicate-audit.md) | Full non-git exact duplicate hash summary. |
| [duplicate-cleanup-verification.md](duplicate-cleanup-verification.md) | Verified duplicate removal evidence, protected families, archive policy, and quarantine result. |
| [save-state.md](save-state.md) | Tracked save slots, screen names, and cleanup plan. |
| [reorganization.md](reorganization.md) | Limited archive reorg decision log. |
| [stale-references.md](stale-references.md) | Old URLs, hard-coded paths, and historical references. |
| [release-scope.md](release-scope.md) | Current branch sharing scope and public-release boundaries. |
| [patch-package-plan.md](patch-package-plan.md) | Branch-delta inventory and restoration-test plan for any future patch/docs-only package. |
| [distribution.md](distribution.md) | Distribution/licensing caution. |
| [security-scan.md](security-scan.md) | Malware scanner guidance, current scanner availability evidence, and scanner-result TSV validation. |
| [gaps.md](gaps.md) | Remaining runtime, cleanup, build, and distribution gaps. |
| [git-handoff.md](git-handoff.md) | Exact branch, remote, pre-push checks, and push-auth status. |
| [push-auth.md](push-auth.md) | Owner-side HTTPS token, GitHub CLI, and SSH options for pushing the branch. |

For a current generated handoff summary, run
`tools/print-share-status.sh` from the repository root.

## Generated Evidence

| Doc | Use it for |
| --- | --- |
| [generated/README.md](generated/README.md) | Map for generated command-output snapshots. |
