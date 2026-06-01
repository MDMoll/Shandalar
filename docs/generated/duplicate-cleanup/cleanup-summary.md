# Verified Duplicate Cleanup Summary

Generated from tracked files in `/Users/mdmoll/Shandalar/Shandalar`. This pass treats SHA-256 equality as duplicate-byte evidence only; path removal still requires install-root or purpose evidence.

## Install-Root Archive Cleanup Delta

| Metric | Before install-root archive cleanup | After cleanup | Delta |
| --- | ---: | ---: | ---: |
| Tracked files scanned | 52033 | 52018 | -15 |
| Duplicate SHA-256 groups | 10797 | 10782 | -15 |
| Files inside duplicate groups | 26071 | 26041 | -30 |
| Redundant files if one copy per hash were kept | 15274 | 15259 | -15 |
| Theoretical duplicate bytes | 454817805 | 351174624 | -103643181 |
| Archive files removed from unsupported package root | 0 | 15 | 15 |
| Archive bytes removed from unsupported package root | 0 | 103643181 | 103643181 |

## Post-Removal Classification Counts

| Classification | Groups |
| --- | ---: |
| `safe-remove` | 0 |
| `safe-remove-after-canonical-policy` | 0 |
| `protected-runtime` | 9081 |
| `protected-asset-pool` | 1399 |
| `protected-tooling` | 6 |
| `protected-package-layout` | 0 |
| `needs-runtime-trace` | 0 |
| `needs-human-policy` | 274 |
| `unknown` | 22 |

## Current Decision

Top-level `Mods/` is the canonical active mod archive root. `Manalink3/` is historical/unsupported for active cleanup purposes, so its exact duplicate archive copies were removed. Runtime-looking files in `Program/`, root, and executable-adjacent `Manalink3/Program/` were left untouched.
