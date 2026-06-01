# Repository Cleanup Plan

This is the current cleanup decision log. It complements
[cleanup-audit.md](cleanup-audit.md), [duplicate-audit.md](duplicate-audit.md),
and [cleanup-removed-files.md](cleanup-removed-files.md).

## What Changed In This Pass

| Action | Result |
| --- | --- |
| Regenerated tracked-only duplicate data | 52,010 tracked files scanned before removal; 10,797 duplicate SHA-256 groups; 433.7 MiB theoretical duplicate bytes. |
| Removed tracked OS/editor junk | `CardArtNew/Thumbs.db`, 524,800 bytes. |
| Recomputed after cleanup | 52,009 tracked files scanned; duplicate group and theoretical duplicate-byte counts unchanged. |
| Updated docs and verifier | Runtime manifest, cleanup docs, inventory notes, generated evidence map, and `tools/verify-share-readiness.sh` now reflect that there are no tracked ignored files expected. |

## Verified Duplicate Cleanup Follow-Up

| Action | Result |
| --- | --- |
| Regenerated tracked duplicate groups with path-purpose classifications | 52,025 tracked files before removal; 10,798 duplicate SHA-256 groups; 436.0 MiB theoretical duplicate bytes. |
| Removed an exact duplicate generated report | `docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv`, 2,344,775 bytes, kept `docs/generated/safe-cleanup/largest-duplicate-groups.tsv`. |
| Recomputed after removal | 52,024 tracked files; 10,797 duplicate SHA-256 groups; 454,817,805 theoretical duplicate bytes. |
| Quarantine-tested before removal | Disposable worktree `/private/tmp/shandalar-duplicate-cleanup-test-verified` moved the candidate into `_QUARANTINE_DUPLICATES/` while the kept report remained present. |
| Rechecked archive duplicates | 15 `Mods/` vs `Manalink3/Mods/` archive pairs were later resolved by install-root policy. |

## Install-Root Layout Cleanup Follow-Up

| Action | Result |
| --- | --- |
| Documented install roots | Added [install-roots.md](install-roots.md), [package-layout-cleanup.md](package-layout-cleanup.md), and generated install-root TSVs. |
| Chose canonical active roots | Repo root, top-level `Program/`, and top-level `Mods/` are active canonical layout. |
| Reclassified `Manalink3/` | Historical/unsupported package root for this cleanup pass, not an active supported install root. |
| Removed duplicate archives from unsupported root | 15 exact duplicate archives removed from `Manalink3/Mods/`, 103,643,181 bytes. |
| Recomputed duplicate data | Duplicate SHA-256 groups dropped from 10,797 to 10,782; theoretical duplicate bytes dropped by 103,643,181. |

## Why Large Duplicate Archives Stayed

| Family | Decision | Evidence |
| --- | --- | --- |
| `Mods/Art/*.7z` vs `Manalink3/Mods/Art/*.7z` | Removed duplicate `Manalink3` copies | Top-level `Mods/Art` is canonical after the install-root decision; `Manalink3/` is historical/unsupported as an active root. |
| `Mods/Rogues/*.7z` vs `Manalink3/Mods/Rogues/*.7z` | Removed duplicate `Manalink3` copies | Same install-root decision as art archives. |
| `Mods/Art/_undo` | `needs-human-decision` | Existing launcher logic defines `_undo` folders for rollback staging. |
| `Program/` vs root or `Statwin/` assets | `needs-smoke-test` or `do-not-remove-runtime` | Both root and `Program/` launch surfaces are relevant, and UI/video/resource files are runtime-looking. |
| Card art duplicate stores | `do-not-remove-runtime` | Art lookup trees may be alternate runtime layouts; exact hash matches do not prove either tree is unused. |

## Next Cleanup Batches

| Batch | Candidate | Required proof |
| --- | --- | --- |
| Save/local state | `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, `MAGIC5`, `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | Human-visible save/load test in a disposable copy, then decide archive-vs-fixture policy. |
| Package-root archival | Remaining `Manalink3/Program/`, `Manalink3/PlayDeckAnalyser/`, and non-archive package files | Decide whether to move the whole historical package under `archive/` or keep it in place as evidence. |
| `_undo` rollback state | `Mods/Art/_undo` and related rollback folders | Prove launcher rollback is obsolete or create a replacement rollback workflow. |
| Runtime duplicate assets | `Program/` vs root/`Statwin`/`Spr1024`/art/sound families | Runtime file-access tracing or launch-copy tests that show the removed path is never needed. |
| Source/tool snapshots | `src/` vs `Program/src/`, updater duplicates | Build/tool audit proving one tree is stale and unused. |

## Current Rule

Do not remove runtime-looking assets, executable-adjacent package files,
source/tooling duplicates, or save/local-state fixtures by hash alone. A
duplicate hash proves the bytes match; it does not prove a legacy path is
unused.
