# Tools

Small repository-maintenance helpers live here. They are not game runtime
files.

| Tool | Purpose |
| --- | --- |
| `audit_codebase.py` | Read-only static/source/tooling audit helper that writes inventories, grep-style reports, and the activated damage-prevention handler inventory under `docs/generated/code-audit/`; inventories cover the tree, while grep-style scans default to focused source/tooling scope unless `--scan-scope all` is supplied. |
| `check-build-prereqs.sh` | Read-only build preflight that reports source-tree blockers and missing local Windows/MinGW-style tools before anyone tries a build or historical copy script. |
| `check-source-snapshot-parity.sh` | Read-only `src/` versus `Program/src/` parity report that treats whole-tree divergence as source-provenance evidence while enforcing exact-match and mirrored safety-marker checks for current source fixes. |
| `check-security-scanner-availability.sh` | Reports scanner-related commands visible on the current machine and explains which are usable for the security gate; it does not run a scan. |
| `create-cleanup-test-copy.sh` | Creates a disposable working-tree copy without `.git` for cleanup launch testing after running `verify-share-readiness.sh`; refuses to overwrite an existing destination and supports `--dry-run` and `--include-git`. |
| `create-git-handoff-bundle.sh` | Creates a Git bundle plus `.sha256` sidecar for handing off the current branch when GitHub push authentication is unavailable; runs `verify-share-readiness.sh` first and supports `--dry-run` and narrowly scoped `--replace`. |
| `create-patch-package.sh` | Creates a binary git patch plus `.sha256` sidecar for patch/docs-only package planning; pass `--verify-apply` to test it in a disposable clone and `--replace` to recreate the current default artifact. |
| `create-security-scan-results-template.sh` | Writes a current `security-scan-results.tsv` template with exact tracked scan-target paths and hashes; it does not run a scanner and placeholders must be replaced before validation can pass. |
| `list-branch-delta.sh` | Lists tracked branch changes relative to `master` as TSV with status, path, coarse kind, byte count, and SHA-256 for review or patch-only release planning; pass `--summary` for a Markdown overview. |
| `list-security-scan-targets.sh` | Lists tracked executable, DLL, archive, and script files with kind, byte count, and SHA-256 so a scanner pass has an exact target inventory. |
| `patch-ai-decision-fallback.py` | Legacy guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; changes the missing/invalid `AiDecisionTime` fallback from 5405 or 540 to 270 only after validating exact preimage bytes, and reports the newer clamp hook as a superseding patched state. |
| `patch-ai-decision-clamp.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; routes `_check_timer_for_ai_speculation` through a small executable cave so missing, invalid, or high `AiDecisionTime` values use 270. |
| `patch-ai-raw-mana-snapshot.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; routes the `_ai_decision_phase` raw-mana snapshot block through a small executable cave so both players' raw-mana rows are saved before AI speculation replaces the opponent row. |
| `patch-ai-player-target-selection.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; skips the generic selector for AI-controlled player-only targets after candidate construction has already filtered and ordered legal targets. |
| `patch-ai-etb-player-target-preselect.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; wraps the Piranha Marsh and Bojuka Bog `comes_into_play()` calls so non-speculating AI preselects the player target during `EVENT_TRIGGER`, before Spell Chain priority. |
| `patch-manalink-generic-damage-prevention-guard.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; routes the generic activated damage-prevention helper through a small executable cave so `GAA_DAMAGE_PREVENTION*` abilities are only offered during `LCBP_DAMAGE_PREVENTION`. |
| `patch-piranha-marsh-trigger-target.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; rewrites Piranha Marsh's ETB trigger target selection to use `pick_player_duh()` so AI/Duh mode directly targets the opponent while normal human choice is preserved. |
| `patch-bojuka-bog-trigger-target.py` | Guarded binary patch helper for root and `Program/` `ManalinkEh.dll`; rewrites Bojuka Bog's ETB trigger target selection to use `pick_player_duh()` for the same AI/Duh player-target trigger class. |
| `patch-magic-coinflip-default.py` | Guarded binary patch helper for root and `Program/` `Magic.exe`; changes the missing-registry `ShowCoinFlips` default from on to off only after validating exact preimage bytes. |
| `print-manual-gameplay-baseline.sh` | Prints a Markdown baseline with current branch, CrossOver bottle settings, launch commands, and runtime hashes for visible gameplay testing. |
| `print-security-scan-baseline.sh` | Prints a Markdown scanner-report baseline with current branch, target counts, priority hashes, and commands to run with a real scanner. |
| `print-share-status.sh` | Prints a current Markdown status report with branch facts, inventory counts, handoff artifact paths, artifact hashes, local scanner-result validation status, and remaining gates; it does not push, scan, or launch the game. |
| `record-manual-gameplay-result.sh` | Updates one environment field or one visible gameplay result row in `docs/manual-gameplay-verification.md`; it does not run the game or decide whether a test passed. |
| `record-security-scan-result.sh` | Records one path or all current tracked scan targets in `security-scan-results.tsv` after a real scanner pass; it requires exact current hashes and `--confirmed-real-scan`. |
| `verify-crossover-mtg-state.sh` | Optional local check for this machine's `MTG` CrossOver bottle: copied runtime hashes including patched Magic executables, Manalink damage-prevention, AI decision-time clamp, AI raw-mana snapshot, Piranha Marsh trigger-target bytes, Bojuka Bog trigger-target bytes, generic AI player-target selector bytes, and AI ETB player-target preselection bytes, root/Program `zlib.dll`, Program adjacent config/font files through `DuelArt/Planeswalker.dat` and the six Program `TT*.ttf` drawcard fonts, Program CardArt drawcardlib assets through `Modern/CardOv_Nyx.png`, representative Magic and Manalink patch bytes, `AiDecisionTime=270`, `ShowCoinFlips=0`, `Window = 2`, app-default `win7`, `Shandalar1440=1440x1080`, and paging-file registry state. |
| `verify-final-share-gates.sh` | Strict completion gate for controlled-maintenance sharing; it runs local share-readiness, reports evidence blockers, requires complete manual gameplay evidence, requires full scanner-result coverage, and confirms `origin/<branch>` matches the local branch tip once those evidence gates pass. It is expected to fail until external evidence exists. |
| `verify-handoff-artifacts.sh` | Verifies the default `/private/tmp` bundle and patch artifacts for the current branch tip, including checksum sidecars, bundle import, and patch tree restoration. |
| `verify-handoff-readiness.sh` | Runs the non-gameplay handoff stack: share-readiness, gameplay/security baseline sanity checks, share-status inventory/status/hash drift checks, cleanup-copy dry-runs for default no-`.git` and `--include-git` modes, bundle/patch dry-runs, optional bundle-import verification, optional default artifact verification, and optional CrossOver bottle-state verification. |
| `verify-install-tree.sh` | Verifies any repo-root-shaped install tree for the current patched runtime hashes, Program adjacent files, representative patch bytes, and config values without requiring Git, CrossOver, or a GUI launch. |
| `verify-manual-gameplay-results.sh` | Validates the manual gameplay checklist and fails until required environment fields are filled and all required visible tests pass; pass `--show-missing` to print the exact missing fields and test IDs. |
| `verify-security-scan-results.sh` | Validates a local scanner-results TSV against the current tracked scan target inventory and hashes; pass `--require-all` before treating the security gate as complete. |
| `verify-share-readiness.sh` | Runs automated checks for clean-tree status, ignored local clutter, generated report/handoff ignore rules, absence of tracked ignored files, Git binary attributes, protected cleanup false positives, tracked fresh-install runtime/support files, patched runtime hashes, install-tree verification, runtime-manifest hashes, representative patch bytes, tracked save/local-state inventory, security-scan target inventory, branch-delta inventory shape, core docs, current top-level CrossOver guidance, maintained-text ASCII, docs index coverage, and local Markdown links. |

Run from the repository root:

```sh
tools/create-cleanup-test-copy.sh --dry-run
python3 tools/audit_codebase.py --out docs/generated/code-audit
python3 tools/audit_codebase.py --out docs/generated/code-audit --scan-scope all
tools/check-build-prereqs.sh --report-only
tools/check-source-snapshot-parity.sh
tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only
tools/create-cleanup-test-copy.sh /private/tmp/shandalar-cleanup-test
tools/create-cleanup-test-copy.sh --include-git /private/tmp/shandalar-cleanup-test-with-git
tools/check-security-scanner-availability.sh
tools/create-git-handoff-bundle.sh --dry-run
tools/create-git-handoff-bundle.sh --replace
tools/create-patch-package.sh --dry-run
tools/create-patch-package.sh --replace --verify-apply
tools/create-security-scan-results-template.sh --output security-scan-results.tsv
tools/list-branch-delta.sh
tools/list-branch-delta.sh --summary
tools/list-security-scan-targets.sh
uv run python tools/patch-ai-decision-fallback.py
uv run python tools/patch-ai-decision-clamp.py
python3 tools/patch-ai-raw-mana-snapshot.py
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
uv run python tools/patch-magic-coinflip-default.py
tools/print-manual-gameplay-baseline.sh
tools/print-security-scan-baseline.sh
tools/print-share-status.sh
tools/record-manual-gameplay-result.sh --test D2 --result "Fail: froze at post-combat Done; screenshot /path/to/screenshot.png"
tools/record-security-scan-result.sh --confirmed-real-scan --path Shandalar.exe --scanner "Windows Defender" --version "VERSION" --date 2026-05-31 --result "Clean" --notes "MpCmdRun.exe custom scan completed"
tools/verify-final-share-gates.sh
tools/verify-handoff-artifacts.sh
tools/verify-handoff-readiness.sh
tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts
tools/verify-install-tree.sh /path/to/Shandalar
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
