# Verified Duplicate Cleanup Summary

Generated from tracked files in `/Users/mdmoll/Shandalar/Shandalar` on the local machine. This pass treats SHA-256 equality as duplicate-byte evidence only; path removal still requires purpose evidence.

## Duplicate Inventory Delta

| Metric | Before removal | After removal | Delta |
| --- | ---: | ---: | ---: |
| Tracked files scanned | 52025 | 52024 | -1 |
| Duplicate SHA-256 groups | 10798 | 10797 | -1 |
| Files inside duplicate groups | 26073 | 26071 | -2 |
| Redundant files if one copy per hash were kept | 15275 | 15274 | -1 |
| Theoretical duplicate bytes | 457162580 | 454817805 | -2344775 |
| Files removed | 0 | 1 | 1 |
| Bytes removed | 0 | 2344775 | 2344775 |

## Post-Removal Classification Counts

| Classification | Groups |
| --- | ---: |
| `safe-remove` | 0 |
| `safe-remove-after-canonical-policy` | 0 |
| `protected-runtime` | 9081 |
| `protected-asset-pool` | 1399 |
| `protected-tooling` | 6 |
| `protected-package-layout` | 15 |
| `needs-runtime-trace` | 0 |
| `needs-human-policy` | 274 |
| `unknown` | 22 |

## Removed File

Removed `docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv` after proving it was an exact duplicate of `docs/generated/safe-cleanup/largest-duplicate-groups.tsv` with SHA-256 `c765aca06fa871fa2b0799a8d8283f93037af49a3a388e2a2326a29063b68b48`. The kept file was rehashed after removal.

## Archive Decision

Archive duplicates under `Manalink3/Mods/` were evaluated but protected because local launcher/package evidence demonstrates that `Manalink3` remains a self-contained package snapshot. The default keep-`Mods/` policy is documented in `archive-policy.md`, but the local evidence override prevents applying it in this pass.
