# Cleanup Removed Files

This records files actually removed during the safe duplicate cleanup pass.
Deletion is deliberately narrow: exact duplicate evidence alone did not justify
removing runtime/package assets.

## Executive Summary

| Metric | Value |
| --- | ---: |
| Files removed | 1 |
| Bytes removed | 524,800 |
| Duplicate archive files removed | 0 |
| Runtime-looking files removed | 0 |
| Git history rewritten | 0 |

## Removed Files

| Removed path | Kept path | Size | SHA-256 | Reason | Evidence | Validation | Risk |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| `CardArtNew/Thumbs.db` | None; generated Windows Explorer thumbnail cache | 524,800 | `d613ed811f078af12887dfb5d056373606c29d036574ee31315c427bf5f101ea` | Tracked OS/editor junk, not card art or game data. | `file` reported `Composite Document File V2 Document`; it was the only tracked OS/editor junk candidate; exact-path references were docs/tools updated in this pass; binary string scan over tracked executables/DLLs found no `Thumbs.db` reference. | `git rm -- CardArtNew/Thumbs.db`; regenerated tracked duplicate summaries; `tools/verify-share-readiness.sh` passes after the verifier update. | Low. The path was inside `CardArtNew/`, so the pass intentionally removed only the thumbnail cache and left all art files untouched. |

## Archive Duplicates Not Removed

The pass rechecked the exact duplicate archive pairs under `Mods/` and
`Manalink3/Mods/`. All 15 sampled pairs are byte-identical, but both
`Manalink_Launcher.cmd` files define `modDir` relative to their own package and
enumerate local `Art` and `Rogues` archives. `Manalink3/` is also documented as
a self-contained package snapshot.

Because of that, these archive pairs were classified as
`needs-human-decision`, not `safe-remove`. Removing one side would be a package
canonicalization decision, not a mechanically safe duplicate cleanup.

## Validation Performed

| Check | Result |
| --- | --- |
| `docs/generated/safe-cleanup/duplicate-summary.tsv` | Recorded tracked-only pre-cleanup duplicate metrics. |
| `docs/generated/safe-cleanup/removal-approved.tsv` | Contains one `safe-remove` row, `CardArtNew/Thumbs.db`. |
| `docs/generated/safe-cleanup/removal-rejected.tsv` | Records duplicate archive/runtime families left untouched. |
| `docs/generated/safe-cleanup/cleanup-size-delta.tsv` | Records 1 file and 524,800 bytes removed; duplicate byte totals unchanged. |
| `tools/verify-share-readiness.sh` | Passes after removal and verifier update. |

## Rollback

To restore the removed file from the parent commit of this cleanup branch:

```sh
git checkout HEAD^ -- CardArtNew/Thumbs.db
```

Then revert the related docs/verifier updates or run:

```sh
git revert <cleanup-commit>
```

Future `Thumbs.db` files remain ignored by `.gitignore`.
