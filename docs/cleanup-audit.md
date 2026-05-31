# Cleanup Audit

The first audit deleted nothing. The limited reorganization pass later moved a
small approved set of generated/local/debug/historical files into `archive/`
with `git mv`. Runtime assets, launch targets, source snapshots, mods, decks,
and art stayed in place.

## Candidate Table

| Path | Category | Reason | Evidence | Confidence | Recommended action |
| --- | --- | --- | --- | --- | --- |
| `archive/generated-local/Duel.GID` | Definitely generated/recreatable | Windows Help `.GID` files are generated index/bookmark files. | `file Duel.GID` reported `MS Windows help Bookmark`; `Duel.hlp` and `Duel.cnt` are present. | High | Archived in limited reorg; keep unless a removal audit proves it disposable. |
| `archive/generated-local/shandalar_dll.log` | Generated log | Looks like runtime/debug output. | `file` reported CSV text; filename is a log. | Medium | Archived in limited reorg; keep as preserved evidence. |
| `archive/debug-evidence/ML_Debug.txt` | Generated debug log | Contains old `D:\Newmagic\...` debug paths. | Original `ML_Debug.txt:3`, `:7`, etc. | Medium | Archived in limited reorg; keep as evidence. |
| `archive/debug-evidence/assertFile.txt` | Generated/debug evidence | Contains `D:\NewMagic\sources\sidlib\lib.c`. | Original `assertFile.txt:2`. | Medium | Archived in limited reorg; keep as evidence. |
| `archive/backups/Rogues_Org_BAK.csv` | Historical backup | Filename marks original backup. | Same line count as `Rogues.csv`, different SHA-256; not referenced by local text search. | Medium | Archived in limited reorg; keep unless a data audit approves removal. |
| `Decks.zip` | Archive duplicate candidate | Deck folders already exist, but archive may be distribution evidence. | `file` reports Zip archive; multiple deck folders exist. | Low | Document, do not delete. |
| `Magicsaves.7z` | Archive | Could contain useful save files. | `file` reports 7-Zip archive. | Low | Keep unless user asks to extract/audit saves. |
| `AllMagicFonts.zip` | Archive | Fonts also exist as `.ttf`/`.otf` files. | `file` reports Zip archive; many font files are present. | Low | Keep or document contents later. |
| `CardArtNew/Thumbs.db` | Generated local-state file inside runtime-like art folder | Windows Explorer thumbnail cache, not card art. | `file CardArtNew/Thumbs.db` reports `Composite Document File V2 Document`; SHA-256 is `d613ed811f078af12887dfb5d056373606c29d036574ee31315c427bf5f101ea`. | High for being generated, medium for moving because it lives inside an art folder | Keep in place until explicit approval to move it with `git mv` into `archive/generated-local/`. |
| `Mods/Art/*.7z` and `Mods/Rogues/*.7z` vs `Manalink3/Mods/...` | Exact duplicate archive pairs | Targeted hash audit over `*.7z`, `*.zip`, and `*.rar` under `Mods/` and `Manalink3/Mods/` found 15 duplicate hashes covering all 30 archive files in that query. | `find Mods Manalink3/Mods ... | shasum -a 256` found pairs such as `Default Sonic 2014.7z`, `Modern Sarlack MtG.7z`, `CardFrames Sonic 2014.7z`, and `Classic Microprose Default.7z`. | High duplicate confidence, medium cleanup confidence | Keep for now; these live in mod/distribution trees, so move only after an approved launch-copy/mod audit. |
| `Decks - Original`, `Decks_original`, `Decks - Harder`, `Decks_alt`, `decks - rawky`, etc. | Exact duplicate deck files plus variant deck sets | Targeted hash audit over the main deck-family folders found 163 duplicate hashes covering 327 of 492 `.dck` files. | Examples include `Decks - Original/0263.dck` matching `Decks_original/0263.dck`, `decks - rawky/0399.dck` matching `decks/0399.dck`, and `Decks - Tim/0056.dck` matching `Decks - revision/0056.dck`. | High duplicate-file confidence, low cleanup confidence | Keep; content duplicates do not prove folder-level redundancy or current runtime references. |
| `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | Save/local player state candidate | Save slots and adjacent map/face/screen-name state are tracked in the root. `Savedescs` and `Screennames/*` contain human-readable names. | See [save-state.md](save-state.md). | Medium cleanup confidence | Keep for now; archive only after a save/load test decides whether they are fixtures or local state. |
| `SVEtool.ini` | Local/default config | Hard-coded `C:\Program Files\Magic\` paths. | `SVEtool.ini:6`, `SVEtool.ini:10`. | Medium | Keep as tool config; document stale defaults. |
| `archive/local-helpers/shandalar_homedoom.bat` | Local helper | Hard-coded `e:\Program Files\Magic` and resolution command. | Original `shandalar_homedoom.bat:1-5`. | Medium | Archived in limited reorg; not main launch guidance. |
| `src/deploy.bat` | Historical deployment script | Hard-coded `c:\magic2k` and `c:\mingw` paths. | `src/deploy.bat:1-44`. | Medium | Keep as build-history evidence; do not run directly. |
| `src/build.pl` | Risky build helper | Runs make and copies output to hard-coded Windows paths. | `src/build.pl:47-53`. | Medium | Patch only in a future build-focused pass. |
| `Statwin/*.tmp` and `Program/statwin/*.tmp` | Runtime-looking UI assets despite `.tmp` suffix | Most are Windows bitmap mask files; they live beside `*.bmp`, `*.avi`, and `statwin.txt` UI resources. Root and Program `statscrn.tmp` differ by SHA-256, so they are not safe duplicate cleanup candidates. | `file Statwin/*.tmp Program/statwin/*.tmp` reports PC bitmap data for the mask files; `shasum -a 256` shows matching root/Program mask hashes but different `statscrn.tmp` hashes. | High keep | Do not archive or ignore by extension; test in a launch copy before any future Statwin cleanup. |
| Root `Shandalar.exe` | Binary duplicate but current CrossOver `MTG` launch target | Same SHA-256 as `Program/Shandalar.exe`; current `MTG` shortcut targets root `C:\Shandalar\Shandalar.exe`, while direct `Program` launch lacks `zlib.dll`. | Hash comparison plus CrossOver shortcut/log evidence. | High keep | Keep; do not dedupe or move without launch-copy testing. |
| Root `Magic.exe` | Suspicious/needs manual inspection | Same PE timestamp/import family as `Program/Magic.exe` but different SHA-256. | Hash comparison showed different values. | Low | Keep and test separately; do not overwrite either copy. |
| `Manalink3/` | Likely packaged duplicate/distribution snapshot | Contains its own `Program/`, `Mods/`, and docs; some sampled mod archives match root `Mods/`. | Directory map plus sampled SHA-256 matches. | Low | Keep as historical packaged distribution until a full hash and launch audit is done. |
| `archive/historical-docs/README (2).txt`, `archive/historical-docs/Readme13.txt`, `archive/historical-docs/Readme132.txt`, `archive/historical-docs/Readme201.txt` | Stale historical docs | Contain old patch, download, and forum references. | See `docs/stale-references.md`. | Medium | Archived in limited reorg; keep as historical source material. |

## Category Coverage

| Requested category | Covered by |
| --- | --- |
| Definitely generated/recreatable | `archive/generated-local/Duel.GID`, generated/log/debug files, and `CardArtNew/Thumbs.db` pending explicit move approval. |
| Likely duplicate | Targeted duplicate hash audit for `Mods/` vs `Manalink3/Mods/` archives, targeted deck-family duplicate hash audit, root `Shandalar.exe`. |
| Likely stale historical backup | `archive/backups/Rogues_Org_BAK.csv`, archived old readme files, archived local helper script. |
| Likely unused by current launch path | Root duplicate candidates and some packaged snapshots, marked low/medium confidence only. |
| Suspicious/needs manual inspection | Root `Magic.exe`, root-vs-Program DLL differences, local machine helper scripts, and `.tmp` files in runtime resource folders. |
| Do-not-touch runtime assets | Listed below. |

## Do-Not-Touch Runtime Assets

| Path/pattern | Why |
| --- | --- |
| `Program/*.exe`, `Program/*.dll` | Direct launch targets and imported DLLs. |
| `Program/*.dat`, `Program/*.res`, `Program/*.csv`, `Program/*.txt`, `Program/*.ini` | Runtime databases, strings, and configs. |
| `Program/CardArt`, `Program/DuelArt`, `Program/DuelSounds`, `Program/Sound`, `Program/SPR*`, `Program/Statwin`, `Program/PlayFace`, `Program/Faces` | Asset folders likely used by UI, cards, sounds, sprites, and faces. |
| `CardArtManalink`, `CardArtNew`, `Cardart`, `Duelart`, `Exp1art`, `Shellart`, `Sound`, `Statwin` | Large art/resource stores. They may look redundant but need runtime tests. |
| `src/`, `Program/src/`, `magic_updater/`, `Program/magic_updater/` | Source/updater snapshots with differences. |

## Quarantine Plan for a Future Approved Pass

1. Create a copy of the repo outside the working tree.
2. Run a baseline launch test for `Program/Shandalar.exe` and `Program/Magic.exe`.
3. Move only high-confidence candidates into `unused-candidates/` in the copy.
4. Re-run launch tests and record exact failures.
5. Only then propose a real repo change.

## Verification Commands

| Purpose | Command |
| --- | --- |
| Check whether a path is referenced | `rg -n --fixed-strings 'FILENAME' .` |
| Compare two files | `shasum -a 256 path1 path2` |
| Find likely generated files | `find . -type f \( -name '*.GID' -o -name '*.log' -o -name '*BAK*' -o -name '*.tmp' \) -print` |
| Full duplicate hash report | `find . -type f -exec shasum -a 256 {} + | sort` |
| Check whether `.tmp` files are actual temp files | `file Statwin/*.tmp Program/statwin/*.tmp && shasum -a 256 Statwin/*.tmp Program/statwin/*.tmp` |

## Targeted Duplicate Hash Audit

These checks were run against the current checkout on 2026-05-31. They are
cleanup evidence only; they do not prove that the duplicate-looking folders are
safe to remove.

| Scope | Command shape | Result |
| --- | --- | --- |
| `Mods/` plus `Manalink3/Mods/` archives | `find Mods Manalink3/Mods -type f \( -name '*.7z' -o -name '*.zip' -o -name '*.rar' \) -print0 | xargs -0 shasum -a 256` | 30 archive files, 15 duplicate hashes, 30 files covered by duplicate pairs. |
| Main deck-family folders | `find 'Decks - Original' Decks_original 'Decks - Harder' Decks_alt 'decks - rawky' decks 'decks - new original' 'Decks - Tim' 'Decks - revision' -type f -name '*.dck' -print0 | xargs -0 shasum -a 256` | 492 `.dck` files, 163 duplicate hashes, 327 files covered by duplicate groups. |

## Archived in Limited Reorg

See [docs/reorganization.md](reorganization.md) for exact paths, commands, and
verification results.
