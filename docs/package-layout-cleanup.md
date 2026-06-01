# Package Layout Cleanup

This is the decision log for applying the install-root cleanup policy.

## Decision

| Topic | Decision | Evidence |
| --- | --- | --- |
| Canonical active layout | Repo root plus top-level `Program/` and top-level `Mods/`. | README and AGENTS launch notes; root launcher uses `%~dp0Program` and `%~dp0Mods`. |
| `Manalink3/` support status | Historical/unsupported install root for this pass. | Its launcher is byte-identical to the root launcher, but active launch docs do not require `Manalink3/`. |
| Duplicate archive policy | Keep top-level `Mods/` copies; remove exact duplicates from `Manalink3/Mods/`. | User policy says local `Mods` enumeration alone does not prove a duplicate install root must be kept. |
| Runtime files | Leave alone. | Active `Program/` and executable-adjacent files still need runtime/file-access proof before any cleanup. |

## Removed Duplicate Archives

All removed archive paths had identical SHA-256 and basename to the kept
top-level `Mods/` copy before removal. The kept copies were rehashed after
`git rm`.

| Removed Path | Kept Path | Size | SHA-256 |
| --- | --- | ---: | --- |
| `Manalink3/Mods/Art/CardFrames Sonic 2014.7z` | `Mods/Art/CardFrames Sonic 2014.7z` | 1,818,701 | `129d5aa8515e1aad57d74b45c3d12ae910c3c7292f960ae4b9b509854d3781f3` |
| `Manalink3/Mods/Art/Default Sonic 2014.7z` | `Mods/Art/Default Sonic 2014.7z` | 34,871,655 | `40918e8c11cc883dc7c96bd6b04f9957cec3c3eff1a505cf5d3d8886c01f9c9b` |
| `Manalink3/Mods/Art/DuelFace_DuelLife Sonic 2013.7z` | `Mods/Art/DuelFace_DuelLife Sonic 2013.7z` | 3,032,147 | `b9a430189d2b0ace7f68182be0dc86273bad964bebe5d2b5a5ba95c43f5ed901` |
| `Manalink3/Mods/Art/DuelTerrain Ozks 2013.7z` | `Mods/Art/DuelTerrain Ozks 2013.7z` | 6,466,888 | `d3d0f2f50b9a8a4d7c40aa357cd53ddf442433fbef266452bc58b17ddd791f3c` |
| `Manalink3/Mods/Art/Duel_DB Microprose 1997.7z` | `Mods/Art/Duel_DB Microprose 1997.7z` | 3,554,931 | `2a3cb186ae5c951230627b4d7d7dcf8199084323721c8952db37fd8c7e505af2` |
| `Manalink3/Mods/Art/Duel_DB_Shell Ozks SolidClean 2014.7z` | `Mods/Art/Duel_DB_Shell Ozks SolidClean 2014.7z` | 3,776,150 | `ecd04be68b1ca32c3930ffa8a99ddd3d13ea24438233bfddc23c226150cdf224` |
| `Manalink3/Mods/Art/Duel_Shell Salbei 2010.7z` | `Mods/Art/Duel_Shell Salbei 2010.7z` | 10,178,595 | `d9471da4723f8f5e3de487e0568513a9ea960c37a2be03b3a737beefb578d655` |
| `Manalink3/Mods/Art/Duel_Shell Sarlack 2013.7z` | `Mods/Art/Duel_Shell Sarlack 2013.7z` | 20,945,536 | `35f8da3703b83edb5ce07b1cd264ef010bb63c07acef9299077fd30701f67965` |
| `Manalink3/Mods/Art/PlayFace Ozks Anime 2014.7z` | `Mods/Art/PlayFace Ozks Anime 2014.7z` | 5,238,509 | `1e7fa8d70193c698444c73c313aeabb4a8a5a35fa9f1fe0fdd454d85fec3e26d` |
| `Manalink3/Mods/Rogues/Classic CirothUngol Ridiculous.7z` | `Mods/Rogues/Classic CirothUngol Ridiculous.7z` | 513,420 | `ebc90eefafaaac95655de33bee10bd8c0b22e018ebd483d93077d7c5b6d1f0e4` |
| `Manalink3/Mods/Rogues/Classic Microprose Default.7z` | `Mods/Rogues/Classic Microprose Default.7z` | 551,248 | `5943267792989f2c6e91537f1fb8a11a7a91d2bc15fe559c50f2123304da7b59` |
| `Manalink3/Mods/Rogues/Classic Ozks Antiques.7z` | `Mods/Rogues/Classic Ozks Antiques.7z` | 2,021,426 | `aa1e81a370101234ee7e1d98484f56bdf06159cd0d4d0296842a6863c97c98d6` |
| `Manalink3/Mods/Rogues/Classic Stassy MagicCardUsers.7z` | `Mods/Rogues/Classic Stassy MagicCardUsers.7z` | 104,764 | `8417dac4a4a6677533c77a0183a0df1376d8d89450f35ccc282d5434fc2c5837` |
| `Manalink3/Mods/Rogues/Modern Ozks 16x9.7z` | `Mods/Rogues/Modern Ozks 16x9.7z` | 4,655,773 | `7b5814a4bb3159cc76ad2ffbf639d2b809faada1c686eb5e4e2beb1c36ad6cb7` |
| `Manalink3/Mods/Rogues/Modern Sarlack MtG.7z` | `Mods/Rogues/Modern Sarlack MtG.7z` | 5,913,438 | `71458236ae8fc41c9b5bc7e73bf4fd9d52270bac5f128b94c4481e25dfb5eaa5` |

## Result

| Metric | Value |
| --- | ---: |
| Archive files removed | 15 |
| Archive bytes removed | 103,643,181 |
| Active runtime files removed | 0 |
| Top-level `Mods/` canonical copies removed | 0 |

## Deferred Work

| Candidate | Reason deferred |
| --- | --- |
| Moving all of `Manalink3/` under `archive/` | This would be a large package-layout move involving executable-adjacent files and should be its own review. |
| Removing `Manalink3/Program/` runtime files | These are executable-adjacent and outside this archive-only pass. |
| Removing `_undo` rollback trees | Launcher logic still names rollback folders. |
| Removing root/`Program`/asset duplicates | Active runtime layout and file access still need proof. |
