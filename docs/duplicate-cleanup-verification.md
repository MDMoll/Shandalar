# Verified Duplicate Cleanup

This pass moves beyond byte-duplicate discovery and records whether a duplicate
path has the same purpose as its matching copy. Hashes prove identical bytes;
they do not prove a legacy lookup path is unnecessary.

## Verified on this machine

Run from `/Users/mdmoll/Shandalar/Shandalar` on the local machine clock,
2026-05-31.

| Check | Result |
| --- | --- |
| Branch | `codex/verified-duplicate-cleanup` |
| Duplicate inventory | `docs/generated/duplicate-cleanup/duplicate-groups.tsv` classifies 10,797 post-removal tracked duplicate SHA-256 groups. |
| Safe duplicate removed | `docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv`, 2,344,775 bytes. |
| Kept canonical copy | `docs/generated/safe-cleanup/largest-duplicate-groups.tsv`, SHA-256 `c765aca06fa871fa2b0799a8d8283f93037af49a3a388e2a2326a29063b68b48`. |
| Quarantine test | Disposable worktree `/private/tmp/shandalar-duplicate-cleanup-test-verified` moved the candidate to `_QUARANTINE_DUPLICATES/`; the kept report remained present and the candidate path was absent. |
| Archive policy | `Manalink3/Mods/` archive duplicates are protected because `Manalink3/` has a package-local launcher, `Program/`, `Mods/`, utility tools, and local archive enumeration. |

## Removed Files

| Removed Path | Kept Path | SHA256 | Size | Evidence | Confidence |
| --- | --- | --- | ---: | --- | --- |
| `docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv` | `docs/generated/safe-cleanup/largest-duplicate-groups.tsv` | `c765aca06fa871fa2b0799a8d8283f93037af49a3a388e2a2326a29063b68b48` | 2,344,775 | Exact SHA-256 duplicate; no maintained references outside generated evidence; disposable worktree quarantine passed; kept file hash reverified after `git rm`. | High |

## Protected Duplicates

| Family | Example Paths | Reason Protected | Evidence Needed |
| --- | --- | --- | --- |
| `Manalink3/Mods/` archives vs root `Mods/` archives | `Manalink3/Mods/Art/Default Sonic 2014.7z`; `Mods/Art/Default Sonic 2014.7z` | `Manalink3/Manalink_Launcher.cmd` sets `modDir` relative to itself and counts local archive files, so the tree is shaped like a self-contained package. | Explicit policy that `Manalink3/` no longer needs to remain standalone, plus a disposable mod-menu/listing test. |
| `_undo` rollback trees | `Mods/Art/_undo/2016_02_12_12_15_32/...` | The launcher defines `undoArt`/`undoPlayDeck`; many `_undo` files are runtime-looking resources. | Rollback workflow audit proving the tree is obsolete or has a replacement. |
| Runtime/resource families | `Program/`, `Statwin/`, `Spr1024/`, root/`Program` binaries and DLLs | Launch and resource lookup paths are still active or unresolved. | Runtime trace or launch-copy evidence. |
| Art/deck pools | `CardArtManalink/`, `CardArtNew/`, deck folders | Duplicate image/deck content can still serve distinct lookup or variant roles. | Runtime/file-access trace and user policy on deck variants. |
| Source/tooling snapshots | `src/`, `Program/src/`, `magic_updater/`, `Program/magic_updater/` | Similar files do not prove equivalent build/tool purpose. | Build/tool audit proving one copy is stale. |

## Generated Evidence

| Path | Contents |
| --- | --- |
| `docs/generated/duplicate-cleanup/duplicate-groups.tsv` | Post-removal tracked duplicate groups, risk flags, and classification. |
| `docs/generated/duplicate-cleanup/removal-candidates.tsv` | The removed generated-report duplicate plus protected archive pairs. |
| `docs/generated/duplicate-cleanup/removed-files.tsv` | Exact removed path, kept path, SHA-256, size, and evidence. |
| `docs/generated/duplicate-cleanup/protected-duplicates.tsv` | Protected family summary and required proof. |
| `docs/generated/duplicate-cleanup/static-reference-results.tsv` | Reference checks for the safe candidate and archive families. |
| `docs/generated/duplicate-cleanup/quarantine-test-results.tsv` | Disposable worktree quarantine result. |
| `docs/generated/duplicate-cleanup/archive-policy.md` | Default archive canonical policy and why it is not applied to `Manalink3/` here. |
| `docs/generated/duplicate-cleanup/cleanup-summary.md` | Before/after duplicate metrics and classification counts. |

## Rollback

Restore the removed generated report from the parent commit if needed:

```sh
git checkout HEAD^ -- docs/generated/safe-cleanup/post-cleanup-largest-duplicate-groups.tsv
```

No Git history rewrite was performed.
