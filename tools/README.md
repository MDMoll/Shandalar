# Tools

Small repository-maintenance helpers live here. They are not game runtime
files.

| Tool | Purpose |
| --- | --- |
| `create-cleanup-test-copy.sh` | Creates a disposable working-tree copy without `.git` for cleanup launch testing after running `verify-share-readiness.sh`; refuses to overwrite an existing destination and supports `--dry-run` and `--include-git`. |
| `create-git-handoff-bundle.sh` | Creates a Git bundle plus `.sha256` sidecar for handing off the current branch when GitHub push authentication is unavailable; runs `verify-share-readiness.sh` first and supports `--dry-run` and narrowly scoped `--replace`. |
| `create-patch-package.sh` | Creates a binary git patch plus `.sha256` sidecar for patch/docs-only package planning; pass `--verify-apply` to test it in a disposable clone and `--replace` to recreate the current default artifact. |
| `create-security-scan-results-template.sh` | Writes a current `security-scan-results.tsv` template with exact tracked scan-target paths and hashes; it does not run a scanner and placeholders must be replaced before validation can pass. |
| `list-branch-delta.sh` | Lists tracked branch changes relative to `master` as TSV with status, path, coarse kind, byte count, and SHA-256 for review or patch-only release planning; pass `--summary` for a Markdown overview. |
| `list-security-scan-targets.sh` | Lists tracked executable, DLL, archive, and script files with kind, byte count, and SHA-256 so a scanner pass has an exact target inventory. |
| `print-manual-gameplay-baseline.sh` | Prints a Markdown baseline with current branch, CrossOver bottle settings, launch commands, and runtime hashes for visible gameplay testing. |
| `print-security-scan-baseline.sh` | Prints a Markdown scanner-report baseline with current branch, target counts, priority hashes, and commands to run with a real scanner. |
| `print-share-status.sh` | Prints a current Markdown status report with branch facts, inventory counts, handoff artifact paths, artifact hashes, local scanner-result validation status, and remaining gates; it does not push, scan, or launch the game. |
| `record-manual-gameplay-result.sh` | Updates one environment field or one visible gameplay result row in `docs/manual-gameplay-verification.md`; it does not run the game or decide whether a test passed. |
| `record-security-scan-result.sh` | Records one path or all current tracked scan targets in `security-scan-results.tsv` after a real scanner pass; it requires exact current hashes and `--confirmed-real-scan`. |
| `verify-crossover-mtg-state.sh` | Optional local check for this machine's `MTG` CrossOver bottle: copied runtime hashes, `Window = 2`, app-default `win7`, `Shandalar1440=1440x1080`, and paging-file registry state. |
| `verify-final-share-gates.sh` | Strict completion gate for controlled-maintenance sharing; it runs local share-readiness, reports evidence blockers, requires complete manual gameplay evidence, requires full scanner-result coverage, and confirms `origin/<branch>` matches the local branch tip once those evidence gates pass. It is expected to fail until external evidence exists. |
| `verify-handoff-artifacts.sh` | Verifies the default `/private/tmp` bundle and patch artifacts for the current branch tip, including checksum sidecars, bundle import, and patch tree restoration. |
| `verify-handoff-readiness.sh` | Runs the non-gameplay handoff stack: share-readiness, gameplay/security baseline sanity checks, share-status inventory/status/hash drift checks, cleanup-copy dry-runs for default no-`.git` and `--include-git` modes, bundle/patch dry-runs, optional bundle-import verification, optional default artifact verification, and optional CrossOver bottle-state verification. |
| `verify-manual-gameplay-results.sh` | Validates the manual gameplay checklist and fails until required environment fields are filled and all required visible tests pass; pass `--show-missing` to print the exact missing fields and test IDs. |
| `verify-security-scan-results.sh` | Validates a local scanner-results TSV against the current tracked scan target inventory and hashes; pass `--require-all` before treating the security gate as complete. |
| `verify-share-readiness.sh` | Runs automated checks for clean-tree status, ignored local clutter, generated report/handoff ignore rules, expected tracked ignored files, Git binary attributes, protected cleanup false positives, patched runtime hashes, runtime-manifest hashes, representative patch bytes, tracked save/local-state inventory, security-scan target inventory, branch-delta inventory shape, core docs, current top-level CrossOver guidance, maintained-text ASCII, docs index coverage, and local Markdown links. |

Run from the repository root:

```sh
tools/create-cleanup-test-copy.sh --dry-run
tools/create-cleanup-test-copy.sh /private/tmp/shandalar-cleanup-test
tools/create-cleanup-test-copy.sh --include-git /private/tmp/shandalar-cleanup-test-with-git
tools/create-git-handoff-bundle.sh --dry-run
tools/create-git-handoff-bundle.sh --replace
tools/create-patch-package.sh --dry-run
tools/create-patch-package.sh --replace --verify-apply
tools/create-security-scan-results-template.sh --output security-scan-results.tsv
tools/list-branch-delta.sh
tools/list-branch-delta.sh --summary
tools/list-security-scan-targets.sh
tools/print-manual-gameplay-baseline.sh
tools/print-security-scan-baseline.sh
tools/print-share-status.sh
tools/record-manual-gameplay-result.sh --test D2 --result "Fail: froze at post-combat Done; screenshot /path/to/screenshot.png"
tools/record-security-scan-result.sh --confirmed-real-scan --path Shandalar.exe --scanner "Windows Defender" --version "VERSION" --date 2026-05-31 --result "Clean" --notes "MpCmdRun.exe custom scan completed"
tools/verify-final-share-gates.sh
tools/verify-handoff-artifacts.sh
tools/verify-handoff-readiness.sh
tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts
tools/verify-manual-gameplay-results.sh --allow-incomplete
tools/verify-manual-gameplay-results.sh --allow-incomplete --show-missing
tools/verify-security-scan-results.sh --allow-missing
tools/verify-share-readiness.sh
```

After a real scanner writes `security-scan-results.tsv`, run
`tools/verify-security-scan-results.sh --require-all` before treating the
security-scan gate as complete.

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

Verifier-created disposable clones under `/private/tmp` are cleaned up
automatically. Default handoff artifacts such as `/private/tmp/*.bundle` and
`/private/tmp/*.patch` are intentionally left in place for sharing until you
remove them.
