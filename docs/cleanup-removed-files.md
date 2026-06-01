# Cleanup Removed Files

This records files actually removed during the safe duplicate cleanup pass.
Deletion is deliberately narrow: exact duplicate evidence alone did not justify
removing runtime/package assets.

## Executive Summary

| Metric | Value |
| --- | ---: |
| Files removed | 17 |
| Bytes removed | 106,512,756 |
| Duplicate archive files removed | 15 |
| Runtime-looking files removed | 0 |
| Git history rewritten | 0 |

## Removed Files

| Removed path | Kept path | Size | SHA-256 | Reason | Evidence | Validation | Risk |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| `CardArtNew/Thumbs.db` | None; generated Windows Explorer thumbnail cache | 524,800 | `d613ed811f078af12887dfb5d056373606c29d036574ee31315c427bf5f101ea` | Tracked OS/editor junk, not card art or game data. | `file` reported `Composite Document File V2 Document`; it was the only tracked OS/editor junk candidate; exact-path references were docs/tools updated in this pass; binary string scan over tracked executables/DLLs found no `Thumbs.db` reference. | `git rm -- CardArtNew/Thumbs.db`; regenerated tracked duplicate summaries; `tools/verify-share-readiness.sh` passes after the verifier update. | Low. The path was inside `CardArtNew/`, so the pass intentionally removed only the thumbnail cache and left all art files untouched. |
| `docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv` | `docs/generated/safe-cleanup/largest-duplicate-groups.tsv` | 2,344,775 | `c765aca06fa871fa2b0799a8d8283f93037af49a3a388e2a2326a29063b68b48` | Exact duplicate generated report; the post-cleanup largest-group table was byte-identical to the kept canonical table. | `shasum -a 256` matched both files; maintained docs/tools had no reference outside generated evidence; the generated report was quarantined successfully in `/private/tmp/shandalar-duplicate-cleanup-test-verified`. | `git rm -- docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv`; kept report hash reverified; generated duplicate cleanup reports updated. | Low. This was a generated evidence artifact, not a runtime file. |
| 15 duplicate archives under `Manalink3/Mods/Art/` and `Manalink3/Mods/Rogues/` | Matching top-level `Mods/Art/` and `Mods/Rogues/` archives | 103,643,181 | See [package-layout-cleanup.md](package-layout-cleanup.md). | `Manalink3/` is now a historical/unsupported package root for active cleanup; top-level `Mods/` is canonical. | Each removed archive had identical basename and SHA-256 to the kept top-level `Mods/` copy before removal. | `git rm -- Manalink3/Mods/...`; kept archive hashes reverified; install-root decision recorded in [install-roots.md](install-roots.md). | Low for active layout. This intentionally reduces standalone completeness of `Manalink3/` rather than touching runtime assets. |

## Archive Duplicate Decision

The earlier file-level audit kept `Manalink3/Mods/` archives because the
package-local launcher can enumerate them. The install-root policy supersedes
that: local enumeration proves internal coherence, not that the repo must keep
two supported package roots. Top-level `Mods/` is now canonical for these
archives, and `Manalink3/` is retained only as historical/package evidence.

## Validation Performed

| Check | Result |
| --- | --- |
| `docs/generated/safe-cleanup/duplicate-summary.tsv` | Recorded tracked-only pre-cleanup duplicate metrics. |
| `docs/generated/safe-cleanup/removal-approved.tsv` | Contains one `safe-remove` row, `CardArtNew/Thumbs.db`. |
| `docs/generated/safe-cleanup/removal-rejected.tsv` | Records duplicate archive/runtime families left untouched. |
| `docs/generated/safe-cleanup/cleanup-size-delta.tsv` | Records 1 file and 524,800 bytes removed; duplicate byte totals unchanged. |
| `docs/generated/duplicate-cleanup/cleanup-summary.md` | Records the verified duplicate report removal: 1 duplicate group and 2,344,775 theoretical duplicate bytes removed in this pass. |
| `docs/generated/duplicate-cleanup/quarantine-test-results.tsv` | Records the disposable worktree quarantine check before removing the generated-report duplicate. |
| `docs/generated/install-root-inventory.tsv` | Records install roots, launchers, local folders, evidence, and status. |
| `docs/generated/install-root-decision.tsv` | Records the keep/archive/remove decision for each root. |
| [package-layout-cleanup.md](package-layout-cleanup.md) | Lists all 15 removed archive paths with kept paths, sizes, and SHA-256 values. |
| `tools/verify-share-readiness.sh` | Passes after removal and verifier update. |

## Rollback

To restore the removed files from the parent commit of the cleanup branch that
deleted them:

```sh
git checkout HEAD^ -- CardArtNew/Thumbs.db
git checkout HEAD^ -- docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv
git checkout HEAD^ -- Manalink3/Mods/Art Manalink3/Mods/Rogues
```

Then revert the related docs/verifier updates or run:

```sh
git revert <cleanup-commit>
```

Future `Thumbs.db` files remain ignored by `.gitignore`.
