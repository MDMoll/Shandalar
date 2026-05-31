# Pending Cleanup Moves

This is the queue for cleanup moves that look reasonable but still need either
explicit approval, launch-copy testing, or both. The goal is to make the repo
cleaner without quietly breaking old runtime layouts.

## Move Candidates

| Candidate | Proposed archive path | Evidence | Required before move |
| --- | --- | --- | --- |
| `CardArtNew/Thumbs.db` | `archive/generated-local/CardArtNew-Thumbs.db` | Windows Explorer thumbnail cache; `file` reports `Composite Document File V2 Document`; SHA-256 `d613ed811f078af12887dfb5d056373606c29d036574ee31315c427bf5f101ea`. | Explicit approval because the source path is inside a runtime-like art folder. |
| `MAGIC5` | `archive/save-state/MAGIC5` | ASCII text derived from `MAGIC5.SVE`; begins with `; Importing Magic.exe.Cards.csv` and `; Loading MAGIC5.SVE data...`. | Approval for save-state cleanup. This one is safer than the binary save slots, but should stay grouped with save evidence. |
| `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce` | `archive/save-state/slots/` | Binary save slots and adjacent map/face state. | Save/load test in a launch copy that proves new game, load game, and save game still work without root save slots. |
| `CSV/MAGIC3/` through `CSV/MAGIC6/` | `archive/save-state/csv-exports/` | Text exports from save inspection. | Decide whether these are public fixtures or just local evidence. |
| Root `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | `archive/save-state/local-player-state/` | Local save descriptions, FaceMaker state, and screen-name profiles with visible names. | Save/load and character creation retest after moving them in a copy. |

## Deferred Package-Tree State

| Candidate | Evidence | Decision |
| --- | --- | --- |
| `Program/Savedescs` | Empty tracked file inside `Program/`. | Do not move separately from the `Program/` runtime tree. Revisit only during a launch-copy cleanup of program-local state. |
| `Manalink3/Program/ScreenNames/` | Contains an active-name file and one screen-name profile inside the packaged `Manalink3/` tree. | Do not move separately from `Manalink3/`; treat as package-snapshot evidence until that tree has its own cleanup decision. |

## Suggested Commands After Approval

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
mkdir -p archive/generated-local archive/save-state
git mv CardArtNew/Thumbs.db archive/generated-local/CardArtNew-Thumbs.db
git mv MAGIC5 archive/save-state/MAGIC5
```

For the larger save-state move, use a launch copy first:

```sh
mkdir -p archive/save-state/slots archive/save-state/csv-exports archive/save-state/local-player-state
git mv MAGIC*.SVE MAGIC*.map MAGIC*.fce archive/save-state/slots/
git mv CSV/MAGIC3 CSV/MAGIC4 CSV/MAGIC5 CSV/MAGIC6 archive/save-state/csv-exports/
git mv Savedescs FaceMostRecent.txt Screennames archive/save-state/local-player-state/
```

## Verification After Any Move

```sh
git status --short --untracked-files=all
git diff --stat
git ls-files -ci --exclude-standard
rg -n "CardArtNew/Thumbs.db|MAGIC5|Savedescs|FaceMostRecent|Screennames|ScreenNames" README.md AGENTS.md docs archive
```

If a manual launch test is part of the move, record the exact launch path,
working directory, Wine/CrossOver or Windows version, and visible result in
[running.md](running.md) or [share-readiness.md](share-readiness.md).
