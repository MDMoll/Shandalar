# Share Readiness

This checklist summarizes what is clean enough to share from the current branch
and what still needs direct proof. It is intentionally practical and
conservative.

## Ready to Share

| Area | Current state | Evidence |
| --- | --- | --- |
| Branch hygiene | Working tree should be clean before pushing. | Run `git status --short --untracked-files=all`; expected output is empty. |
| Primary README | New users have a short starting point and launch-target map. | [../README.md](../README.md) points Windows, Wine, CrossOver, and future agents to the right docs. |
| Agent guardrails | Runtime assets and patched binaries have explicit protection notes. | [../AGENTS.md](../AGENTS.md) documents do-not-move rules, archive policy, and patch verification commands. |
| Limited reorg | Obvious non-runtime clutter was moved out of the root without deletion. | [reorganization.md](reorganization.md) records exact `git mv` paths and verification. |
| Runtime patches | Active `Shandalar.exe`, `FaceMaker.exe`, `Magic.exe`, and `ManalinkEh.dll` changes are documented with offsets and hashes. | See [running.md](running.md), [magic-exe.md](magic-exe.md), and focused bug docs under [bugs/](bugs/). |
| Runtime manifest | Active patched files and protected references have a compact hash manifest for review and scanning. | [runtime-manifest.md](runtime-manifest.md). |
| Detailed local evidence | Long command/result evidence is in docs instead of the primary README. | [verified-on-this-machine.md](verified-on-this-machine.md). |
| Cleanup evidence | Remaining candidates are inventoried with confidence instead of being deleted. | [cleanup-audit.md](cleanup-audit.md) and [file-inventory.md](file-inventory.md). |
| Cleanup test plan | Risky remaining moves have a disposable launch-copy test plan and a helper that can dry-run or create a checked full-copy target without overwriting existing paths. | [cleanup-launch-copy-test.md](cleanup-launch-copy-test.md), `tools/create-cleanup-test-copy.sh`. |
| Git binary hygiene | Root `.gitattributes` marks legacy runtime/resource/media/archive/save formats as binary. | `git check-attr --all -- Shandalar.exe Program/Magic.exe Cards.dat Statwin/statscrn.tmp README.md`. |
| Distribution caution | The docs distinguish a cleaned maintenance branch from public redistribution permission. | [distribution.md](distribution.md). |
| Release scope | Current branch sharing is scoped as controlled maintenance, not public redistribution. | [release-scope.md](release-scope.md). |
| Patch-only planning | Branch-delta and binary-patch helpers exist, but no patch package is committed or release-approved yet. | [patch-package-plan.md](patch-package-plan.md), `tools/list-branch-delta.sh`, `tools/create-patch-package.sh`. |
| Git handoff | Branch, remote, current pushed state, earlier auth failure, owner-side auth paths, Git bundle/patch fallback, checksum sidecars, and artifact verification are recorded. | [git-handoff.md](git-handoff.md), [push-auth.md](push-auth.md), `tools/create-git-handoff-bundle.sh`, `tools/create-patch-package.sh`, `tools/verify-handoff-artifacts.sh`. |
| Completion audit | The original cleanup/share goal is mapped to current proof and final gates. | [completion-audit.md](completion-audit.md). |
| Current share status | A single read-only report prints the active branch, commit, inventory counts, expected handoff artifact paths, artifact hashes when present, local scanner-result validation status, manual gameplay evidence status, and remaining gates without claiming gameplay or scan proof. | `tools/print-share-status.sh`. |
| Automated non-gameplay checks | Repo share-readiness and handoff checks are repeatable without launching the game, including ignored-local-clutter checks, generated report/handoff ignore checks, protected cleanup false positives, runtime-manifest hash checks, tracked save/local-state inventory, security-scan target inventory, optional scanner-result TSV validation, manual-gameplay checklist validation, branch-delta inventory shape, current top-level CrossOver guidance, maintained-text ASCII, docs index coverage, local Markdown link validation, evidence-baseline sanity checks, share-status inventory/status/hash drift checks, cleanup-copy dry-runs for default no-`.git` and `--include-git` modes, bundle/patch dry-runs, optional bundle-import verification, and optional default artifact verification. | `tools/verify-share-readiness.sh`, `tools/verify-handoff-readiness.sh`, `tools/verify-security-scan-results.sh`, `tools/verify-manual-gameplay-results.sh`. |
| Final gate command | A strict controlled-maintenance verifier exists; it now verifies the pushed branch and is expected to fail until manual gameplay evidence is complete and scanner-result coverage is full. | `tools/verify-final-share-gates.sh`. |
| Manual gameplay evidence helpers | Visible gameplay testing still has to be done by a human, but the branch/hash/CrossOver baseline can be printed repeatably before testing, and one-row results can be recorded without hand-editing Markdown tables. | [manual-gameplay-verification.md](manual-gameplay-verification.md), `tools/print-manual-gameplay-baseline.sh`, `tools/record-manual-gameplay-result.sh`. |
| Local helper scope | CrossOver helper scripts are separated from runtime files. | [../local/README.md](../local/README.md). |
| Optional local CrossOver state check | The current `MTG` bottle state can be checked without launching the game, but this is intentionally not a general share-readiness gate. | `tools/verify-crossover-mtg-state.sh`. |
| Generated evidence scope | Long command-output snapshots are mapped as evidence, not primary docs. | [generated/README.md](generated/README.md). |

## Do Not Claim Yet

| Claim | Why not yet |
| --- | --- |
| The game is fully gameplay-verified on CrossOver. | Visible manual testing still needs to confirm character creation, same-arrow map stop, save/load, duel stability, and all five starting colors; see [manual-gameplay-verification.md](manual-gameplay-verification.md). |
| Native Windows behavior is verified. | No native Windows launch pass has been recorded in this checkout. |
| `Shandalar.exe --help` is documented from actual output. | Local attempts did not capture help text or dialog output. |
| Duplicate assets are safe to remove. | [duplicate-audit.md](duplicate-audit.md) now measures the full non-git duplicate graph, but no launch-copy removal test has chosen canonical runtime/package paths. |
| Tracked save files are ready for public release. | [save-state.md](save-state.md) documents save slots and screen-name state that should be reviewed after save/load testing. |
| All generated/local clutter has been archived. | `CardArtNew/Thumbs.db` and save-state/export files remain tracked in place pending explicit approval and testing; see [cleanup-move-plan.md](cleanup-move-plan.md). |
| Public redistribution is approved. | [distribution.md](distribution.md) records that no repository-level license file was found and bundled rightsholder/trademark notices are present. |
| A patch/docs-only public package exists. | [patch-package-plan.md](patch-package-plan.md) records helper commands for temporary patch artifacts and tree-apply verification, but no package artifact has been committed, visibly runtime-tested, scanner-verified, or release-approved. |
| Binaries are malware-scanned. | [security-scan.md](security-scan.md) records that ClamAV is not available on this machine and `spctl` did not produce a useful Windows PE safety result; no named malware scanner result is recorded. Use `tools/print-security-scan-baseline.sh` to prepare the evidence template before or after a real scan. |
| `src/` rebuilds the shipped binaries. | `make -n` still hits missing/generated build inputs and toolchain assumptions. |

## Push Checklist For New Commits

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
tools/print-share-status.sh
git log --oneline -10
tools/verify-share-readiness.sh
git push -u origin codex/shandalar-crossover-updates
```

The current branch has already been pushed; rerun this checklist after new
commits. If the push fails with an HTTPS credential error, authenticate GitHub
locally and rerun the same push command. See [git-handoff.md](git-handoff.md)
for the exact remote and [push-auth.md](push-auth.md) for HTTPS token, GitHub
CLI, and SSH options.

## First Manual Gameplay Checklist

Use [manual-gameplay-verification.md](manual-gameplay-verification.md) for the
canonical visible test plan. Record results in that file, [running.md](running.md),
or a focused bug note, keeping verified observations separate from inferences.
