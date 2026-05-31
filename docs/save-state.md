# Save-State Files

This repo currently includes Shandalar save slots, derived save exports, and
related local state. They may be useful for testing, but they also make the
checkout less clean for public sharing unless intentionally kept.

## Tracked Save-State Inventory

| Path pattern | Count | Evidence | Share note |
| --- | ---: | --- | --- |
| `MAGIC3.SVE` through `MAGICd.SVE` | 11 | `file` reports binary `data`; each `.SVE` is about 169K. | Likely save slots. Keep until a save/load test proves they can be archived or replaced with fixtures. |
| `MAGIC3.map` through `MAGICd.map` | 11 | `file` reports binary `data`; sizes range from about 15K to 23K. | Likely adjacent map state for the save slots. Move only with matching `.SVE`/`.fce` files. |
| `MAGIC3.fce` through `MAGICd.fce` | 11 | `file` reports binary `data`; sizes range from about 6K to 8.4K. | Likely adjacent face/character state for the save slots. Move only with matching `.SVE`/`.map` files. |
| `MAGIC5` | 1 | ASCII text; begins with `; Importing Magic.exe.Cards.csv` and `; Loading MAGIC5.SVE data...`. | Derived save/deck export, not a launch target. Safe-looking cleanup candidate, but keep with save-state evidence until the save cleanup pass. |
| `CSV/MAGIC3/` through `CSV/MAGIC6/` | 32 | Text exports such as `MAGIC3.SVE.Journal.csv`, `MAGIC3.SVE.Towns.csv`, and `MAGIC3.SVE.OwnedCards.csv`. | Derived save-inspection data, useful evidence but not required to launch. |
| `Savedescs` | 1 | Plain text save descriptions, including names such as `la fin`, `blanc`, and `antievil alliance`. | Local/player-facing save labels; review before public release. |
| `FaceMostRecent.txt` | 1 | ASCII text with `10, fb11`. | Local FaceMaker/character helper state. |
| `Screennames/Activename.dat` | 1 | Hex dump begins with `CirothUngol`. | Local active screen-name state; review before public release. |
| `Screennames/*.scn` | 2 | `Player.scn` begins with `Lizzy`; `CirothUngol.scn` begins with `CirothUngol`. | Local screen-name profiles; review before public release. |

## Current Decision

| Decision | Reason |
| --- | --- |
| Keep save-state files in place for now. | Save/load behavior is not yet visibly verified after the runtime patches, and these files may be useful fixtures for manual testing. |
| Do not add broad ignore rules for `MAGIC*.SVE`, `MAGIC*.map`, or `MAGIC*.fce` yet. | These files are already tracked; ignoring them now would make tracked-local-state behavior less obvious. |
| Treat save-state cleanup as a separate approved move. | Moving save slots affects testability and should happen with an explicit before/after launch or save-load check. |

## Future Cleanup Plan

1. Record hashes for every `MAGIC*.(SVE|map|fce)` file plus the extensionless
   `MAGIC5` export.
2. Launch the game from a copy of the checkout and verify whether new game,
   load game, and save game work without the existing slots.
3. If the game works without them, move the tracked save-state files with
   `git mv` to `archive/save-state/` or a clearly named fixture folder.
4. If representative saves are useful, keep one fixture set and archive the
   rest with exact path/hashes in [reorganization.md](reorganization.md).

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git ls-files | rg '(^|/)(MAGIC[0-9a-d]\.(SVE|map|fce)|MAGIC5$|CSV/MAGIC[0-9a-d]/|Savedescs|Screennames/|FaceMostRecent\.txt)'
file MAGIC5 MAGIC*.SVE MAGIC*.map MAGIC*.fce CSV/MAGIC*/* Screennames/* FaceMostRecent.txt Savedescs
shasum -a 256 MAGIC5 MAGIC*.SVE MAGIC*.map MAGIC*.fce FaceMostRecent.txt Screennames/*
```
