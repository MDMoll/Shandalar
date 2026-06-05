# Runtime Path Audit

This audit focuses on path and working-directory risk. It did not launch the
game.

Generated evidence:
[generated/code-audit/runtime-file-references.tsv](generated/code-audit/runtime-file-references.tsv),
[generated/code-audit/hardcoded-paths.tsv](generated/code-audit/hardcoded-paths.tsv),
and
[generated/code-audit/binary-runtime-strings.txt](generated/code-audit/binary-runtime-strings.txt).

## Canonical Layout

| Layout | Status | Evidence |
| --- | --- | --- |
| Repo root plus top-level `Program/` | verified canonical active layout | README, AGENTS, install-root docs, launcher path behavior |
| Top-level `Mods/` | verified canonical mod archive/staging root | `Manalink_Launcher.cmd` uses `%~dp0Mods` |
| `Manalink3/` | verified historical/unsupported active root | install-root cleanup decision |
| Direct `Program/Shandalar.exe` in CrossOver `MTG` | loader, config, font, drawcardlib asset, and card-data gaps observed so far are mitigated, visible path still needs retest | earlier docs record missing `Program\zlib.dll`; later visible/logged runs showed missing Program CardArt assets, `Program\Manalink.ini`, `Program\DuelArt\Modern.dat`, `Program\DuelArt\Planeswalker.dat`, and six Program `TT*.ttf` font lookups. Later visible fatals exposed older 15718-record Program card-data files: first `Cards.dat`/`Rarity.dat`, then `DBInfo.dat`. This checkout and the local `MTG` copied install now include matching root/Program copies for those observed dependencies. |

## Runtime String Findings

| Reference | Evidence | Interpretation |
| --- | --- | --- |
| `Cards.dat`, `DBInfo.dat`, `Rarity.dat` | strings/source in `DeckDLL.dll`/`deckdll.cpp`; visible Program-path `raw_cards_data[-1]` Hornet fatal and then generic `Cards.dat, DBInfo.dat or Rarity.dat` fatal with older Program data | basename lookup; working directory matters, and all three record counts must match |
| `DuelSounds`, `Sound` | strings in `Magic.exe`, `DeckDLL.dll`, `cdtools.dll` | relative runtime folders |
| `%s\FaceMaker.exe` | strings in root and Program `Magic.exe` families | helper path is computed from a base directory |
| `FaceMaker-nores.exe` | string in root `ManalinkEx.dll`/related runtime evidence | preserve no-resolution helper until handoff is fully understood |
| `zlib.dll` | strings in image/statwin-related DLLs | adjacent DLL layout matters |
| `Program\CARDART\ManaSymbols.pic` | visible 2026-06-03 `drawcardlib.dll` dialog during direct Program-path launch | Program top-level CardArt assets must be adjacent to the copied Program runtime path |
| `Program\CARDART\modern\Triggering.png` | bounded 2026-06-03 exact-path `MTG` log after the first CardArt copy | Program CardArt subfolder assets must also match the copied Program runtime path |
| `Program\CARDART\planeswalker\LoyaltyBase.png` | bounded 2026-06-03 exact-path `MTG` log after the `Triggering.png` copy | Planeswalker subfolder assets are also part of the direct Program card-rendering path |
| `Program\CARDART\modern\CardOv_Nyx.png` | bounded 2026-06-04 exact-path `MTG` log after the loyalty-image copy | The Modern frame config maps `CardOv_*Nyx` entries to a generic Nyx overlay file |
| `Program\Manalink.ini` | bounded 2026-06-04 exact-path `MTG` log after the `CardOv_Nyx.png` copy | `Deckdll` builds `Manalink.ini` from the executable directory, so direct Program launches need an adjacent Program copy |
| `Program\DuelArt\Modern.dat` | bounded 2026-06-04 exact-path `MTG` log after the `Program\Manalink.ini` copy | Drawcardlib frame config opens the default Modern frame config relative to the Program `DuelArt` directory |
| `Program\DuelArt\Planeswalker.dat` | bounded 2026-06-04 exact-path `MTG` log after the `Modern.dat` copy | Drawcardlib frame config also opens the Planeswalker frame config relative to the Program `DuelArt` directory |
| `Program\TT*.ttf` | bounded 2026-06-04 exact-path `MTG` log after the `Modern.dat` copy and `drawcardlib/config.c` hardcoded font paths | Drawcardlib adds private card-rendering fonts from the executable base directory |
| `Program\Cards.dat`, `Program\DBInfo.dat`, and `Program\Rarity.dat` | visible 2026-06-04 Program-path fatals `Bad raw_cards_data[-1].card() building Hornet: expected 15835, got -1` and `Fatal: Couldn't find Cards.dat, DBInfo.dat or Rarity.dat`, plus local header/hash inspection and `/tmp/shandalar-mtg-program-post-dbinfo-visible-retest-cx.log` | Direct Program Shandalar startup reads the Program card-data trio; the older 15718-record Program files were too short or mismatched, so the Program trio now matches the 16818-record root trio and opens from the Program path in a bounded log |
| `D:\NewMagic\...` | FaceMaker/source assertion strings | likely compile-time/debug path unless file tracing proves access |

## Path Risks

| Risk | Status | Evidence | Next Step |
| --- | --- | --- | --- |
| Wrong working directory for `Cards.dat`/art/sound | verified risk | basename and relative strings | launch exact paths from their intended folders only |
| Root vs `Program` `Magic.exe` confusion | verified risk | different SHA-256 and launch paths | keep separate test rows and patch verification |
| `Program/zlib.dll` copied-install drift | mitigated for local `MTG` by static hash and loader smoke | repo and local copied install now have matching `Program/zlib.dll`; older `MTG` log failed before copied install had it | verify in any other copied install, then run one exact-path visible retest |
| `Program` adjacent config/font/card-data copied-install drift | mitigated for local `MTG` by static hash and bounded log/visible fatal evidence | repo and local copied install now have matching `Program/Manalink.ini`, Program `DuelArt` configs through `Planeswalker.dat`, six Program `TT*.ttf` font files, and `Program/Cards.dat`, `Program/DBInfo.dat`, plus `Program/Rarity.dat` matching root; exact-path logs exposed the config/font gaps, visible fatals exposed the older Program card-data trio, and the latest bounded log opened the full Program card-data trio plus `Shandalar.ini` without the earlier fatal strings | rerun direct Program-path visible retest and capture the next result |
| `Program/CardArt` copied-install drift | mitigated for local `MTG` by static hash and bounded log | repo and local copied install now have matching Program CardArt files listed in [runtime-manifest.md](runtime-manifest.md), through `Modern/CardOv_Nyx.png`; bounded logs also opened the tracked `Planeswalker/CardOv_Nyx.png` and loyalty image copies | rerun direct Program-path visible retest and capture the next result |
| Launcher mutates local `Program/` and `Mods/` | verified | `Manalink_Launcher.cmd` uses `%~dp0` and deletes/copies local files | use cleanup test copies |
| Hardcoded historical `c:\magic2k` scripts | mitigated | `src/build.pl`, `src/deploy.bat` | `src/build.pl` legacy copies are opt-in; `src/deploy.bat` destructive path requires explicit confirmation |

## Layout Questions Still Open

| Question | Evidence Needed |
| --- | --- |
| Can `Program/Shandalar.exe` be made a supported active path? | Run exact-path visible launch tests from `C:\Shandalar\Program`; local `MTG` now passes matching static checks for `Program/zlib.dll`, adjacent Program config/font files through `Program/DuelArt/Planeswalker.dat` and six `TT*.ttf` files, `Program/Cards.dat`/`Program/DBInfo.dat`/`Program/Rarity.dat`, and Program CardArt assets observed so far through `Modern/CardOv_Nyx.png`. |
| Which `Magic.exe` should be canonical? | Visible root and Program launch comparison with hashes recorded. |
| Are any remaining `Manalink3/` files required by active root? | File tracing or launcher/source references from root layout. |
| Which FaceMaker helper name is actually spawned in all new-game paths? | Manual character-creation test plus process/log evidence. |
