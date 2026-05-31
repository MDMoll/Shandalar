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
| `Mods/Art/*.7z` vs `Manalink3/Mods/Art/*.7z` | Likely duplicate archives | Some sampled archives have identical SHA-256. | `Default Sonic 2014.7z` matched across both trees. | Medium for sampled files, low for full tree. | Build a full hash report before quarantine. |
| `Mods/Rogues/*.7z` vs `Manalink3/Mods/Rogues/*.7z` | Likely duplicate archives | Sampled archive matched SHA-256. | `Modern Sarlack MtG.7z` matched across both trees. | Medium for sampled files, low for full tree. | Build a full hash report before quarantine. |
| `Decks - Original`, `Decks_original`, `Decks - Harder`, `Decks_alt`, `decks - rawky`, etc. | Likely duplicate or variant deck sets | Many repeated `.dck` filenames across deck folders. | Duplicate filename counts show many deck names repeated 10 times. | Low | Keep; compare content and runtime references first. |
| `SVEtool.ini` | Local/default config | Hard-coded `C:\Program Files\Magic\` paths. | `SVEtool.ini:6`, `SVEtool.ini:10`. | Medium | Keep as tool config; document stale defaults. |
| `archive/local-helpers/shandalar_homedoom.bat` | Local helper | Hard-coded `e:\Program Files\Magic` and resolution command. | Original `shandalar_homedoom.bat:1-5`. | Medium | Archived in limited reorg; not main launch guidance. |
| `src/deploy.bat` | Historical deployment script | Hard-coded `c:\magic2k` and `c:\mingw` paths. | `src/deploy.bat:1-44`. | Medium | Keep as build-history evidence; do not run directly. |
| `src/build.pl` | Risky build helper | Runs make and copies output to hard-coded Windows paths. | `src/build.pl:47-53`. | Medium | Patch only in a future build-focused pass. |
| Root `Shandalar.exe` | Binary duplicate but current CrossOver `MTG` launch target | Same SHA-256 as `Program/Shandalar.exe`; current `MTG` shortcut targets root `C:\Shandalar\Shandalar.exe`, while direct `Program` launch lacks `zlib.dll`. | Hash comparison plus CrossOver shortcut/log evidence. | High keep | Keep; do not dedupe or move without launch-copy testing. |
| Root `Magic.exe` | Suspicious/needs manual inspection | Same PE timestamp/import family as `Program/Magic.exe` but different SHA-256. | Hash comparison showed different values. | Low | Keep and test separately; do not overwrite either copy. |
| `Manalink3/` | Likely packaged duplicate/distribution snapshot | Contains its own `Program/`, `Mods/`, and docs; some sampled mod archives match root `Mods/`. | Directory map plus sampled SHA-256 matches. | Low | Keep as historical packaged distribution until a full hash and launch audit is done. |
| `archive/historical-docs/README (2).txt`, `archive/historical-docs/Readme13.txt`, `archive/historical-docs/Readme132.txt`, `archive/historical-docs/Readme201.txt` | Stale historical docs | Contain old patch, download, and forum references. | See `docs/stale-references.md`. | Medium | Archived in limited reorg; keep as historical source material. |

## Category Coverage

| Requested category | Covered by |
| --- | --- |
| Definitely generated/recreatable | `archive/generated-local/Duel.GID`, generated/log/debug files, and `CardArtNew/Thumbs.db` pending explicit move approval. |
| Likely duplicate | Sampled `Mods/` vs `Manalink3/Mods/` archives, root `Shandalar.exe`. |
| Likely stale historical backup | `archive/backups/Rogues_Org_BAK.csv`, archived old readme files, archived local helper script. |
| Likely unused by current launch path | Root duplicate candidates and some packaged snapshots, marked low/medium confidence only. |
| Suspicious/needs manual inspection | Root `Magic.exe`, root-vs-Program DLL differences, local machine helper scripts. |
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

## Archived in Limited Reorg

See [docs/reorganization.md](reorganization.md) for exact paths, commands, and
verification results.
