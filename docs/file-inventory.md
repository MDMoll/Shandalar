# File Inventory

Generated from local inspection on 2026-05-30. Counts include tracked and
working tree files visible in the checkout at the time of the inventory pass.
The later limited reorganization moved selected root files into `archive/` and
added a few docs without changing runtime launch targets or asset folders. A
follow-up start-color assertion investigation added `Program/FaceMaker.exe` as
a copy of the already tracked `Program/FaceMaker-nores.exe`, plus FaceMaker
support files from `Manalink3/Program/`. A later CrossOver pass established
that bottle `MTG` launches root `C:\Shandalar\Shandalar.exe` by default; direct
`C:\Shandalar\Program\Shandalar.exe` previously failed there because
`Program\zlib.dll` was absent. This checkout and the local `MTG` copied install
now include `Program/zlib.dll` as a byte-for-byte copy of root `zlib.dll`.
A later visible direct-Program launch reached `drawcardlib.dll` and reported
missing `C:\Shandalar\Program\CARDART\ManaSymbols.pic`; the repo and copied
install now also include the Program adjacent config/font files and CardArt
assets observed so far, and later bounded logs identified and resolved the missing
`Program/CardArt/Modern/Triggering.png` and
`Program/CardArt/Planeswalker/LoyaltyBase.png` lookups. A 2026-06-04 bounded
log then identified and resolved the missing `Program/CardArt/Modern/CardOv_Nyx.png`
lookup; follow-up logs identified and resolved `Program/Manalink.ini`, Program
`DuelArt` configs through `Planeswalker.dat`, the six hardcoded Program
`TT*.ttf` drawcard fonts, and older Program card-data files. The latest bounded
Program-path log opened the Program card-data trio, `shandalar.dll`, and
`Shandalar.ini` without the earlier fatal strings; the copied bottle Program
path still needs an exact visible retest.
A historical CrossOver pass tried app-default desktop/`Version=win8`
for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`, and verified Shandalar
main menu plus direct FaceMaker startup, but did not prove a durable fix. The
current `MTG` retest setting is app-default `Version=win7` with virtual desktop
`Shandalar1440=1440x1080`. A later binary patch to root and
`Program/Shandalar.exe` passed the reported start-color crash point in
CrossOver smoke testing from the repo path, the fresh bottle-local
`C:\Shandalar` path, and the older `MTG` shortcut path. A later follow-up
patched the active root and `Program` FaceMaker helpers at their own
`CreateDIBSection` wrapper; the `*-nores.exe` files remain preserved as
no-resolution helpers, and a later visible S2 run logged
`FaceMaker-nores.exe /S` while reaching the adventure map. Later Shandalar
follow-ups patched the name-entry default buffer, name-editor bypass/fallback,
same-arrow adventure-map stop behavior, MagSnd update-message compatibility
path, the minimal WinMM timer-callback compatibility path, the MagSnd
init-disable path, and the MCIWndCreateA disable path,
changing the active Shandalar hash from the hSection-only
`73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9` through the
name-entry-only `bd784cc248d08455270a6bfae5004ead8f9723d8017f8db152add113e8d3a9db`
and the name-seed-plus-movement value `155a668c72867bd1274410eb05ca05fbb7bd9bed843b42d1583ea536805a4aaf` to the current combined-patch
`ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b`.

## Summary

Git internals such as `.git/objects/pack/*.pack` are excluded from
working-tree runtime inventories. They are analyzed separately because they
describe Git history/storage, not game files.

| Metric | Value |
| --- | --- |
| Total files from `find . -type f` | 52,000 at the original inventory point; later cleanup passes removed tracked junk/generated/duplicate archive files and added docs. |
| Largest top-level directory by size | `.git` at about 1.1G |
| Largest non-git content directories | `CardArtManalink` about 565M, `Program` about 341M, `Mods` about 219M, `Manalink3` about 118M after duplicate archive cleanup |
| Dominant file types | `.dck`, `.jpg`, `.pic`, `.png`, `.spr`, `.ogg`, `.bmp`, `.wav`, `.c` |

## Top Directories by File Count

| Directory | File count |
| --- | ---: |
| `CardArtManalink` | 19,363 |
| `Manalink3` | 11,934 |
| `Mods` | 8,087 |
| `Playdeck` | 4,283 |
| `CardArtNew` | 3,628 |
| `Program` | 1,546 |
| `Cardart` | 382 |
| `src` | 320 |
| `Exp1art` | 287 |
| `Duelsounds` | 159 |
| `Duelart` | 157 |
| `Playface` | 154 |
| `Statwin` | 120 |
| `Sound` | 111 |

## Top Directories by Size

| Directory | Approx size |
| --- | ---: |
| `.git` | 1.1G |
| `CardArtManalink` | 565M |
| `Program` | 341M |
| `Mods` | 219M |
| `Manalink3` | 118M after duplicate archive cleanup |
| `Statwin` | 113M |
| `CardArtNew` | 104M |
| `Sound` | 92M |
| `Cardart` | 81M |
| `Duelart` | 39M |
| `Exp1art` | 29M |
| `Duelsounds` | 23M |
| `magic_updater` | 18M |
| `src` | 16M |
| `archive` | Small; preserved generated/local/debug/historical files from the limited reorg. |

## Extension Counts

| Extension | Count | Likely role |
| --- | ---: | --- |
| `.dck` | 23,991 | Deck files. |
| `.jpg` | 23,045 | Card/mod art. |
| `.pic` | 1,109 | Legacy game image/resource files. |
| `.png` | 718 | Card frames/icons/modern art resources. |
| `.spr` | 679 | Legacy sprite resources. |
| `.ogg` | 485 | Mod sound assets. |
| `.bmp` | 378 | UI/art resources. |
| `.wav` | 305 | Game sound assets. |
| `.c` | 300 | Source. |
| `.avi` | 143 | Video resources. |
| `.pl` | 107 | Perl patch/updater scripts. |
| `.txt` | 81 | Docs/config/string tables. |
| `.csv` | 51 | Card/deck/rules data. |
| `.dat` | 48 | Runtime data. |
| `.asm` | 45 | Assembly source/patch code. |
| `.dll` | 35 | Runtime or helper DLLs. |
| `.exe` | 32 | Game/tool executables. |

## Large Files

| Path | Size | Notes |
| --- | ---: | --- |
| `.git/objects/pack/pack-*.pack` | 1.1G | Git object pack, not runtime content. |
| `Mods/Art/Default Sonic 2014.7z` | 33M | Canonical top-level mod archive after duplicate `Manalink3/Mods/Art/Default Sonic 2014.7z` was removed. |
| `Cardart/Medart.cat` | 30M | Card art catalog, likely runtime-critical for some paths. |
| `Shandalar.dll` | 17M | Root runtime DLL. |
| `Program/Shandalar.dll` | 14M | Program runtime DLL. |
| `magic_updater/Manalink.csv` | 13M | Updater/card data. |
| `Program/magic_updater/Manalink.csv` | 13M | Nested updater/card data. |
| `Cards.dat` | 7.6M | Root runtime card data. |
| `Program/Cards.dat` | 7.0M | Program runtime card data. |
| `ManalinkEh.dll` | 6.3M | Root Manalink DLL. |
| `Program/ManalinkEh.dll` | 5.9M | Program Manalink DLL. |

## Likely Generated or Local-State Files

| Path | Evidence | Confidence |
| --- | --- | --- |
| `archive/generated-local/Duel.GID` | `file` reported `MS Windows help Bookmark`; `Duel.hlp` is the actual help file. | High |
| `archive/generated-local/shandalar_dll.log` | Filename and `file` output identify a CSV-style log. | Medium |
| `archive/debug-evidence/ML_Debug.txt` | Contains old `D:\Newmagic\...` source/debug paths. | Medium |
| `archive/debug-evidence/assertFile.txt` | Contains one old assertion source path. | Medium |
| `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, extensionless `MAGIC5`, `Savedescs`, `Program/Savedescs`, `FaceMostRecent.txt`, `Screennames/`, `Manalink3/Program/ScreenNames/` | Tracked save slots, one derived ASCII save/deck export, and local player/screen-name state. Root `Savedescs` is plain text, `Program/Savedescs` is empty, and screen-name files contain visible names in hex dumps. See [save-state.md](save-state.md). | Medium cleanup confidence |
| `CardArtNew/Thumbs.db` | Windows Explorer thumbnail cache; `file` reported `Composite Document File V2 Document`; removed in the safe duplicate cleanup pass after exact-path and binary-string reference checks. | Removed |
| `Statwin/*.tmp` and `Program/statwin/*.tmp` | These look suspicious by extension, but most report as Windows bitmap mask files and live in UI resource folders. Root and Program mask hashes match, while `statscrn.tmp` differs across the two trees. | High keep confidence |
| `Mods/Art/_undo` | Launcher script defines `_undo` folders for mod rollback staging. | Medium |

## Cleanup False Positives

| Path/pattern | Evidence | Cleanup confidence |
| --- | --- | --- |
| `MENUBAK.PIC`, `WINBAK*.PIC`, `WORLBAK1.PIC`, and matching `Program/` copies | `file` reports data; root and `Program/` copies match by SHA-256 pair; `.PIC` is a legacy runtime resource extension in this tree. | High keep confidence despite backup-looking names. |
| `FaceMaker-Original.exe`, `FaceMaker-nores.exe`, `Program/FaceMaker-nores.exe` | Preserved executables for the FaceMaker patch history. The 2026-05-31 visible S2 run also logged `FaceMaker-nores.exe /S`, so the no-resolution helper is runtime-relevant evidence, not just a reference. Hashes are recorded in [bugs/create-dibsection-after-color.md](bugs/create-dibsection-after-color.md). | High keep confidence until a character-creation verification pass chooses otherwise. |

## Duplicate Observations

| Observation | Evidence | Cleanup confidence |
| --- | --- | --- |
| Full non-git duplicate audit found a large exact-duplicate surface. | [duplicate-audit.md](duplicate-audit.md) records the older full non-git scan plus the later tracked-only safe cleanup pass. The tracked-only pass removed one OS cache file and left duplicate runtime/package families untouched. | High duplicate confidence, low cleanup confidence until launch-copy tests choose canonical package/runtime paths. |
| `Program/Shandalar.exe` and root `Shandalar.exe` are identical. | Same active SHA-256: `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b`; pre-MCI callback/MagSnd hash was `17f7af843fd2fd5424e7d36d547f4315d20fdfa840fb5050a96ab9a727a181f6`; hSection-only interim hash was `73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`; name-entry-only interim hash was `bd784cc248d08455270a6bfae5004ead8f9723d8017f8db152add113e8d3a9db`; name-seed-plus-movement hash was `155a668c72867bd1274410eb05ca05fbb7bd9bed843b42d1583ea536805a4aaf`; DIB/name/movement hash was `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; original pre-patch hash was `82c9b659dd131097b29931f0ed266c91d560103bc864d7eb6b806691d0dc9739`. | Medium for dedupe planning, not deletion; adjacent DLL/assets still differ, and copied CrossOver bottle installs may still have older unpatched copies. |
| Active `Program/FaceMaker.exe` and root `FaceMaker.exe` are identical to each other but no longer identical to the no-resolution helper copies. | Active SHA-256: `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`; no-resolution/Korath SHA-256: `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`. The difference is the 11-byte `CreateDIBSection hSection = NULL` patch at file offset `0x5f40`; a visible S2 run still observed `FaceMaker-nores.exe /S`. | Keep active and no-resolution helpers until full character-creation testing chooses a canonical helper. |
| `Program/FaceData.txt`, `Program/FaceButtons.txt`, and `Program/FaceArt/` match `Manalink3/Program/` after the fix. | `cmp` for text files and `diff -qr` for art directory. | Keep as runtime support for `Program/FaceMaker.exe`. |
| `Program/Magic.exe` and root `Magic.exe` differ. | Different SHA-256 values. | Do not dedupe without launch comparison. |
| `Program/zlib.dll`, adjacent Program config/font/card-data files, and Program CardArt assets now match the local runtime evidence observed so far. | `shasum -a 256 zlib.dll Program/zlib.dll` reports `9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90` for both files; the local `MTG` copied install has the same root/Program zlib hash. The later missing-asset/config/font findings are addressed by `Program/Manalink.ini`, Program `DuelArt` configs through `Planeswalker.dat`, the six hardcoded Program `TT*.ttf` files, and the Program CardArt files listed in [runtime-manifest.md](runtime-manifest.md), through `Modern/CardOv_Nyx.png`, in both the repo and copied install. Later Program card-data fatals are addressed by matching `Program/Cards.dat`, `Program/DBInfo.dat`, and `Program/Rarity.dat`; the latest bounded Program-path log opened those plus `shandalar.dll` and `Shandalar.ini` without the earlier fatal strings. | Treat `Program/Shandalar.exe` as a deferred alternate CrossOver path until visibly tested from the exact Program working directory. |
| `Statwin/*.tmp` and `Program/statwin/*.tmp` are mostly exact pairs, despite the temp-looking extension. | SHA-256 hashes match for the bitmap mask files; `statscrn.tmp` differs between root and Program. | Do not dedupe or archive by extension. |
| `Mods/` and `Manalink3/Mods/` contained exact duplicate archives. | A targeted SHA-256 audit over `*.7z`, `*.zip`, and `*.rar` in both trees found 15 duplicate hashes covering all 30 archive files in that query; [install-roots.md](install-roots.md) made top-level `Mods/` canonical and those 15 `Manalink3/Mods/` archive copies were removed. | Resolved for duplicate archives; remaining package/runtime files are still protected. |
| Deck files repeat across many deck folders. | A targeted SHA-256 audit over 492 `.dck` files in the main deck-family folders found 163 duplicate hashes covering 327 files. | High duplicate-file confidence, low cleanup confidence because content duplicates do not prove folder redundancy. |
| `src/` and `Program/src/` are similar but not identical. | `src/Makefile` and `Program/src/Makefile` have different SHA-256 and content. | Do not dedupe casually. |

## Archive After Limited Reorg

| Archive folder | Preserved files |
| --- | --- |
| `archive/generated-local/` | `Duel.GID`, `shandalar_dll.log` |
| `archive/debug-evidence/` | `ML_Debug.txt`, `assertFile.txt` |
| `archive/historical-docs/` | `README (2).txt`, `Readme13.txt`, `Readme132.txt`, `Readme201.txt` |
| `archive/historical-links/` | `Gatheringnet.url`, `Microprose.url` |
| `archive/local-helpers/` | `shandalar_homedoom.bat` |
| `archive/backups/` | `Rogues_Org_BAK.csv` |

## Likely Runtime-Critical Families

Do not remove without a launch-copy test:

| Family | Examples |
| --- | --- |
| Executables/DLLs | root `Shandalar.exe`, root `Magic.exe`, root DLLs, `Program/Shandalar.exe`, `Program/Magic.exe`, `Program/FaceMaker.exe`, `Program/*.dll` |
| Runtime data | `Program/Cards.dat`, `Program/DBInfo.dat`, `Program/Rarity.dat`, `Program/Text.res` |
| Art/resources | `Program/CardArt`, `Program/DuelArt`, `Program/Exp1Art`, `Program/ShellArt`, `Program/SPR*`, `Program/Statwin` |
| Sound/video | `Program/Sound`, `Program/DuelSounds`, `.avi`, `.wav`, `.ogg` |
| Decks/faces | `Program/decks`, `Playdeck`, `Decks*`, `Faces`, `PlayFace` |
| Config/string tables | `Shandalar.ini`, `config.txt`, `Menus.txt`, `ManaLink.txt`, `UIStrings.txt`, `prompts*.txt`, `Program/FaceData.txt`, `Program/FaceButtons.txt` |

## Likely Documentation, Source, and Tool Families

| Family | Examples | Notes |
| --- | --- | --- |
| Current docs | `README.md`, `AGENTS.md`, `docs/`, `archive/README.md` | Maintained entry points for this cleanup/documentation pass. |
| Historical docs | `archive/historical-docs/`, `ManaLink.txt`, `Program/ManaLink.txt`, `Manalink3/Program/ReadMe.txt` | Useful evidence, but not current launch guidance by default. |
| Source and patch tooling | `src/`, `Program/src/`, `src/patches/` | Similar but not proven identical; do not dedupe casually. |
| Updater tooling | `magic_updater/`, `Program/magic_updater/` | Perl/CSV updater snapshots. |
| Utility tools | `PlayDeckAnalyser/`, `Editor/`, `Facemaker/`, root helper executables | Tool role varies; inspect before moving. |
