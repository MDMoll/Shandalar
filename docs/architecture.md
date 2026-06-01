# Repository Architecture

This is a practical map, not a clean-room reconstruction.

## High-Level Map

| Area | Likely role | Evidence level |
| --- | --- | --- |
| Root executable/assets | Current repo-root Shandalar launch surface and copied CrossOver `MTG` launch surface. | Verified by shortcut target, logged `C:\Shandalar\Shandalar.exe` startup, root DLL/assets, root `Magic.exe` access, and the patched repo `Shandalar.exe` start-color smoke test. |
| `Program/` | Manalink launcher runtime bundle and alternate Shandalar layout. | Verified by executables, imported local DLLs, launcher scripts, and asset folders; direct `MTG` Shandalar launch currently lacks `Program\zlib.dll`. |
| `src/` | Source and patch tooling for DLL/card-engine work. | Verified by Makefiles, C/C++/ASM files, and patch scripts. Build not complete here. |
| `magic_updater/` | Perl/CSV card data updater. | Verified by scripts and large CSV inputs. |
| `Mods/` | Canonical active mod archives and mod staging tree. | Verified by `Manalink_Launcher.cmd`, `.7z` archives, and [install-roots.md](install-roots.md). |
| `Manalink3/` | Historical/unsupported Manalink 3 package snapshot. | Verified by nested `Program/`, `Mods/`, and launcher; duplicate archives were removed after top-level `Mods/` was selected as canonical. |
| `PlayDeckAnalyser/` | Separate deck analysis utility. | Verified by readme and config. |
| Art/resource folders | Runtime game art and resource stores. | Verified by file extensions, counts, and strings references. |
| `archive/` | Preserved generated/local/debug/historical files moved out of the root. | Verified by limited reorg decision log and `git mv` paths. |

## Runtime Relationship

```mermaid
flowchart TD
    A["Repo root runtime surface"] --> C["Root Shandalar.exe"]
    A --> M["Root Magic.exe"]
    A --> R["Root DLLs, Cards.dat, art, sound, deck, resource folders"]
    A --> B["Program/ Manalink and alternate runtime bundle"]
    B --> P["Program Shandalar.exe"]
    B --> D["Program Magic.exe"]
    C --> E["Root Drawcardlib.dll, Deckdll.dll, CdTools.dll, CardArtLib.dll"]
    P --> PE["Program Drawcardlib.dll, Deckdll.dll, CdTools.dll, CardArtLib.dll"]
    D --> F["ManalinkEh.dll, ManalinkEx.dll, Deckdll.dll, Drawcardlib.dll"]
    B --> G["Cards.dat, DBInfo.dat, Rarity.dat, Text.res"]
    B --> H["Art, sound, deck, face, sprite folders"]
    A --> I["src/ source and patch scripts"]
    I --> F
    I --> E
    A --> J["Top-level Mods/ canonical mod archive material"]
    J --> B
    A --> L["Manalink3/ historical package evidence"]
    A --> K["archive/ preserved non-runtime evidence"]
```

## Important Relationships

| Relationship | What is verified | What is inferred or unknown |
| --- | --- | --- |
| Root `Shandalar.exe` and `Program/Shandalar.exe` | Same active SHA-256 in this checkout and copied `MTG`/`Shandalar-Win8-Test` installs: `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`. Both include the `CreateDIBSection hSection = NULL` compatibility patch, the `mPlayer` default-name seed, the name-editor bypass/fallback, and the same-arrow movement-stop code cave. Current `MTG` shortcut targets root `C:\Shandalar\Shandalar.exe`, and local bottle copies were patched with backups. | Either path may still behave differently because adjacent DLLs/assets differ; direct `Program` Shandalar currently fails in `MTG` due missing `zlib.dll`; full character creation and movement-stop behavior still need visible manual gameplay verification. |
| Root `Magic.exe` and `Program/Magic.exe` | Same PE timestamp and imports, different SHA-256; root Shandalar opens root `Magic.exe` in logged startup. | The embedded data or patches differ; test both only with exact path recorded. |
| Root `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Both are patched for the shared Samite/Femeref/Kithkin damage-prevention handler. Root patch site is `0x3bb035`, SHA-256 `6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb`; `Program/` patch site is `0x381a25`, SHA-256 `7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517`. | The DLLs differ by path and hash, so copy/test the one adjacent to the exact `Magic.exe` path being launched. Copied CrossOver installs need their DLLs updated separately from the repo. |
| `Program/FaceMaker.exe`, `FaceMaker-nores.exe`, and `Program/FaceMaker-nores.exe` | `Program/FaceMaker-nores.exe` still matches the known Korath/no-resolution helper. Active `Program/FaceMaker.exe` and root `FaceMaker.exe` are patched at file offset `0x5f40`, hash to `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`, and differ from the no-resolution helper by 11 bytes. Direct patched root FaceMaker launch in `MTG` rendered without the DIB assertion, and a later visible S2 run logged `FaceMaker-nores.exe /S` while reaching the map. | User testing says the original FaceMaker swap alone did not fix the start-color assertion; the exact Shandalar-to-FaceMaker helper selection still needs more testing before any helper can be removed. |
| `Manalink_Launcher.cmd` and `Program/Magic.exe` | Launcher sets `mlDir` to `Program` and starts `Magic.exe`. | Full launcher menu was not run. |
| `src/patches/*` and shipped binaries | Many patch scripts state they update `Magic.exe` or `Shandalar.exe` in-place. | Current shipped binaries were not traced back to a reproducible patch sequence. |
| `magic_updater/` and CSV/dat files | Scripts and CSV files are present. | No updater command was run in this pass. |
| `Manalink3/` and root `Mods/` | The formerly duplicated archives had identical SHA-256 in both places; top-level `Mods/` is now canonical. | Remaining `Manalink3/Program/` and package files are historical evidence and not active root layout. |

## Major Folder Roles

| Folder | Role |
| --- | --- |
| `Program/CardArt`, `Program/DuelArt`, `Program/Exp1Art`, `Program/ShellArt`, `Program/Statwin`, `Program/SPR*` | Runtime images, sprites, card frames, shell art, and UI resources. |
| `Program/DuelSounds`, `Program/Sound` | Duel and adventure sound assets. |
| `Program/decks`, `Playdeck`, `decks*`, `Decks*` | Deck data collections. |
| `CardArtManalink`, `CardArtNew`, `Cardart` | Large card-art/resource stores; likely not all loaded by one executable path, but not safe to delete without tests. |
| `Classic`, `Editor`, `Facemaker`, `Icons`, `CSV` | Tools, legacy data, or supporting resources. |
| `Manalink`, `ManaLink`, `Manalink3` | Manalink-related data and historical packaged distribution evidence. Case and spacing matter. |
| `archive/generated-local`, `archive/debug-evidence`, `archive/historical-docs`, `archive/historical-links`, `archive/local-helpers`, `archive/backups` | Preserved evidence/history from the limited reorganization pass. These are not current launch paths. |

## Open Questions

| Question | Next test |
| --- | --- |
| Which exact files are required for root and `Program` Shandalar minimal launch? | Use a copy/quarantine test outside the repo, not deletion in-place. |
| Does the patched `Shandalar.exe` fully resolve the start-color assertion and follow-up name/movement issues during real gameplay? | Run a visible new-game flow in `MTG` or `Shandalar-Win8-Test` through character creation, default-name acceptance, same-arrow map stop, save/load, and one duel; the local bottle Shandalar copies are already patched. |
| Does the patched `ManalinkEh.dll` resolve the Femeref Healer duel freeze? | Copy the patched DLLs into the active copied CrossOver install, then retest the blocker/Femeref Healer combat scenario. |
| Which `Magic.exe` copy should be the canonical one? | Test root and `Program/` copies separately, record SHA-256 and behavior. |
| Can `src/` rebuild `ManalinkEh.dll` or other DLLs today? | Restore/generate `src/card_id.h`, install MinGW/yasm/dlltool/objcopy, then build in a throwaway branch. |
| What does `Shandalar.exe --help` display? | Manual UI/log capture on Windows or CrossOver. |
