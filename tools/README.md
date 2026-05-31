# Tools

Small repository-maintenance helpers live here. They are not game runtime
files.

| Tool | Purpose |
| --- | --- |
| `create-cleanup-test-copy.sh` | Creates a disposable full-checkout copy for cleanup launch testing after running `verify-share-readiness.sh`; refuses to overwrite an existing destination and supports `--dry-run`. |
| `create-git-handoff-bundle.sh` | Creates a Git bundle plus `.sha256` sidecar for handing off the current branch when GitHub push authentication is unavailable; runs `verify-share-readiness.sh` first and supports `--dry-run`. |
| `list-security-scan-targets.sh` | Lists tracked executable, DLL, archive, and script files with kind, byte count, and SHA-256 so a scanner pass has an exact target inventory. |
| `print-manual-gameplay-baseline.sh` | Prints a Markdown baseline with current branch, CrossOver bottle settings, launch commands, and runtime hashes for visible gameplay testing. |
| `print-security-scan-baseline.sh` | Prints a Markdown scanner-report baseline with current branch, target counts, priority hashes, and commands to run with a real scanner. |
| `verify-crossover-mtg-state.sh` | Optional local check for this machine's `MTG` CrossOver bottle: copied runtime hashes, `Window = 2`, app-default `win7`, `Shandalar1440=1440x1080`, and paging-file registry state. |
| `verify-handoff-readiness.sh` | Runs the non-gameplay handoff stack: share-readiness, gameplay/security baseline sanity checks, cleanup-copy dry-run, bundle dry-run with checksum command coverage, optional bundle-import/checksum verification, and optional CrossOver bottle-state verification. |
| `verify-share-readiness.sh` | Runs automated checks for clean-tree status, ignored local clutter, scan-output ignore rules, expected tracked ignored files, Git binary attributes, protected cleanup false positives, patched runtime hashes, runtime-manifest hashes, representative patch bytes, tracked save/local-state inventory, security-scan target inventory, core docs, maintained-text ASCII, docs index coverage, and local Markdown links. |

Run from the repository root:

```sh
tools/create-cleanup-test-copy.sh --dry-run
tools/create-cleanup-test-copy.sh /private/tmp/shandalar-cleanup-test
tools/create-git-handoff-bundle.sh --dry-run
tools/list-security-scan-targets.sh
tools/print-manual-gameplay-baseline.sh
tools/print-security-scan-baseline.sh
tools/verify-handoff-readiness.sh
tools/verify-share-readiness.sh
```

`verify-crossover-mtg-state.sh` is intentionally local-machine specific. Do not
make general share-readiness depend on the user having the `MTG` CrossOver
bottle. On this Mac, run it separately with:

```sh
tools/verify-crossover-mtg-state.sh
```

During an in-progress edit from the repository root, use
`ALLOW_DIRTY=1 tools/verify-share-readiness.sh` to skip only the clean-tree
check.

If you intentionally have ignored local files such as `.DS_Store` during local
work, use `ALLOW_IGNORED_LOCAL=1 tools/verify-share-readiness.sh` to skip only
that check.
