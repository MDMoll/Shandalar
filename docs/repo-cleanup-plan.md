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

## Why Large Duplicate Archives Stayed

| Family | Decision | Evidence |
| --- | --- | --- |
| `Mods/Art/*.7z` vs `Manalink3/Mods/Art/*.7z` | `needs-human-decision` | Exact duplicate archives, but both root and `Manalink3` launchers enumerate their local `Mods/Art` archives. Removing either side could make that package layout incomplete. |
| `Mods/Rogues/*.7z` vs `Manalink3/Mods/Rogues/*.7z` | `needs-human-decision` | Same package-layout issue as art archives. |
| `Mods/Art/_undo` | `needs-human-decision` | Existing launcher logic defines `_undo` folders for rollback staging. |
| `Program/` vs root or `Statwin/` assets | `needs-smoke-test` or `do-not-remove-runtime` | Both root and `Program/` launch surfaces are relevant, and UI/video/resource files are runtime-looking. |
| Card art duplicate stores | `do-not-remove-runtime` | Art lookup trees may be alternate runtime layouts; exact hash matches do not prove either tree is unused. |

## Next Cleanup Batches

| Batch | Candidate | Required proof |
| --- | --- | --- |
| Save/local state | `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, `MAGIC5`, `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | Human-visible save/load test in a disposable copy, then decide archive-vs-fixture policy. |
| Package archive canonicalization | Duplicate archive pairs under `Mods/` and `Manalink3/Mods/` | Decide whether `Manalink3/` must remain self-contained. If not, update launcher/docs and test mod install/list behavior in a copy. |
| `_undo` rollback state | `Mods/Art/_undo` and related rollback folders | Prove launcher rollback is obsolete or create a replacement rollback workflow. |
| Runtime duplicate assets | `Program/` vs root/`Statwin`/`Spr1024`/art/sound families | Runtime file-access tracing or launch-copy tests that show the removed path is never needed. |
| Source/tool snapshots | `src/` vs `Program/src/`, updater duplicates | Build/tool audit proving one tree is stale and unused. |

## Current Rule

Do not remove runtime-looking assets, package-tree archives, source/tooling
duplicates, or save/local-state fixtures by hash alone. A duplicate hash proves
the bytes match; it does not prove a legacy path is unused.
