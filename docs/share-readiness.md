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
| Cleanup evidence | Remaining candidates are inventoried with confidence instead of being deleted. | [cleanup-audit.md](cleanup-audit.md) and [file-inventory.md](file-inventory.md). |
| Local helper scope | CrossOver helper scripts are separated from runtime files. | [../local/README.md](../local/README.md). |
| Generated evidence scope | Long command-output snapshots are mapped as evidence, not primary docs. | [generated/README.md](generated/README.md). |

## Do Not Claim Yet

| Claim | Why not yet |
| --- | --- |
| The game is fully gameplay-verified on CrossOver. | Visible manual testing still needs to confirm character creation, same-arrow map stop, save/load, duel stability, and all five starting colors. |
| Native Windows behavior is verified. | No native Windows launch pass has been recorded in this checkout. |
| `Shandalar.exe --help` is documented from actual output. | Local attempts did not capture help text or dialog output. |
| The repository is free of duplicate assets. | Targeted duplicate audits found exact duplicates, but no full-repo duplicate graph or launch-copy removal test has been completed. |
| Tracked save files are ready for public release. | [save-state.md](save-state.md) documents save slots and screen-name state that should be reviewed after save/load testing. |
| Binaries are malware-scanned. | [security-scan.md](security-scan.md) is a reporting template; no named scanner result is recorded. |
| `src/` rebuilds the shipped binaries. | `make -n` still hits missing/generated build inputs and toolchain assumptions. |

## Push Checklist

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
git log --oneline -10
git push -u origin codex/shandalar-crossover-updates
```

If the push fails with an HTTPS credential error, authenticate GitHub locally and
rerun the same push command.

## First Manual Gameplay Checklist

| Test | Record |
| --- | --- |
| Launch root `Shandalar.exe` from the full checkout or copied `C:\Shandalar` tree. | Exact path, working directory, Windows/Wine/CrossOver version. |
| Start a new game for each color. | Whether character creation reaches the map with default `Player`. |
| Adventure map movement. | Whether pressing the same arrow after a step stops movement. |
| Enter a duel and play several turns. | Whether `Done`, `Trigger`, and `Decline` remain clickable. |
| Femeref/Samite/Kithkin damage-prevention scenario. | Card names, phase, and whether the prompt freezes. |
| Declared attacker undo. | Whether clicking a declared attacker before Done removes it cleanly. |

Record results in [running.md](running.md) or a focused bug note, keeping
verified observations separate from inferences.
