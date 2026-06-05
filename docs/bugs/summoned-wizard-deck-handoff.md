# Summoned Wizard Deck Handoff

## Symptom

On 2026-06-05, the local CrossOver `MTG` playthrough reported a repeatable
world-map encounter freeze: a witch summons an undead/dark knight, then the duel
screen renders only the two player backgrounds and a blank left sidebar. The
coin flip, decks, and normal duel controls never appear, requiring a forced quit
of `Shandalar.exe`.

## Evidence

Static string inspection of `Shandalar.exe` and `Program/Shandalar.exe` shows
five hardcoded summoned-wizard deck paths:

```text
decks\0016.dck
decks\0283.dck
decks\0150.dck
decks\0076.dck
decks\0102.dck
```

Before the fix, the tracked root `decks/` copies of those five files were
modernized variant decks with `.v...` marker sections and 72-76 numeric card
rows if parsed naively. The matching `Program/decks/` copies were plain legacy
handoff decks, with no `.v...` sections. The reported dark-knight encounter maps
to `decks\0016.dck`, whose root copy contained 76 numeric rows before the fix
while `Program/decks/0016.dck` contained a plain 60-card Undead Knight deck.

An existing CrossOver winewrapper process was still active during the
investigation, so no new visible runtime launch was attempted. This fix is based
on static path/deck evidence plus the existing root/Program install-root model,
not manual gameplay proof.

## Fix

The tracked root copies of the five hardcoded summoned-wizard decks now mirror
their existing `Program/decks/` legacy copies:

| Deck | Opponent | Root total | Variant markers |
| --- | --- | ---: | ---: |
| `decks/0016.dck` | Undead Knight | 60 | 0 |
| `decks/0283.dck` | Crusader | 60 | 0 |
| `decks/0150.dck` | Merfolk Shaman | 60 | 0 |
| `decks/0076.dck` | Elvish Magi | 56 | 0 |
| `decks/0102.dck` | Goblin Warlord | 60 | 0 |

`tools/verify-install-tree.sh` now checks that each root deck matches its
`Program/decks/` counterpart, contains no `.v...` markers, and has the expected
legacy card count.

`tools/verify-crossover-mtg-state.sh` also checks the local CrossOver `MTG`
copied install for the same five root/`Program` deck pairs, so a stale installed
copy of `decks/0016.dck` cannot pass the normal bottle-state verifier.

## Verification

Run the static install-tree check from the repo root or a copied install:

```sh
tools/verify-install-tree.sh .
tools/verify-crossover-mtg-state.sh
```

Use the save-deck inspector when the copied install is current but a specific
playthrough still freezes during the world-map handoff:

```sh
tools/inspect-shandalar-save-decks.py MAGIC*.SVE
tools/inspect-shandalar-save-decks.py --crossover-bottle MTG --check-nonempty-decks
tools/inspect-shandalar-save-decks.py --crossover-bottle MTG --inactive-basics
```

On 2026-06-05, a bounded root direct-duel smoke used:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-root-ep0016-dark-knight-cx.log --debugmsg +seh,+file,+process "C:\Shandalar\Shandalar.exe" --e 0016 --p 0016
```

It printed `Stand-alone duel: "decks/0016.dck" vs. "decks/0016.dck"`, opened
root `C:\Shandalar\Magic.exe`, and opened `C:\Shandalar\decks\0016.dck` after
normal fallback misses for bare `0016` paths. Targeted scans found no
Hornet/card-data fatal, page fault, DIB assertion, unhandled-exception, or
coin-flip dialog strings before manual cleanup. The cleanup tail includes
`wineserver crashed`, so this remains bounded handoff evidence rather than
visible first-turn proof.

On a later 2026-06-05 recurrence after the deck-file sync, the local `MTG`
install verifier still passed, ruling out stale installed summoned-wizard deck
files. Direct `Magic.exe /start3,1` did not faithfully reproduce the world-map
handoff and did not open `MAGIC3.SVE` in the bounded log, so that path is not a
valid stand-alone reproduction.

The live `MTG` `MAGIC3.SVE` did show a concrete save-state anomaly:

```text
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/MAGIC3.SVE: entries=53 deck1=38 deck2=0 deck3=0 terminator=0x14f4 status=LOW(deck1=38) flags=0x01:38 0x08:15
```

The tracked repo save slots all had deck 1 at 40 or higher, and
`PlayDeckAnalyser/PDAnalyser.ini` records `MinDeckSize = 40`, so the most
concrete hypothesis is that this parked witch/knight save had a below-minimum
campaign deck and the summoned-duel handoff did not recover cleanly.

With no `Shandalar.exe` or `Magic.exe` process running, the live save was backed
up to:

```text
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/MAGIC3.before-dark-knight-under40-repair.SVE
```

Then exactly two existing inactive Plains flag bytes were changed from `0x08` to
deck 1 `0x01`:

```text
0x1492: 08 -> 01
0x1496: 08 -> 01
```

Post-repair verification:

```text
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/MAGIC3.SVE: entries=53 deck1=40 deck2=0 deck3=0 terminator=0x14f4 status=OK flags=0x01:40 0x08:13
```

Follow-up source and runtime hardening now keeps this class of bad save from
being written again through the integrated deckbuilder: `src/deck/deckdll.cpp`
refuses to switch away from or close Shandalar/editor deck views while deck 1 is
below 40, or while any nonempty alternate player deck is below 40. `DeckDLL.dll`
and `Program/Deckdll.dll` were rebuilt and deployed in both the repo and local
`MTG` bottle at SHA-256
`5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0`;
`tools/verify-crossover-mtg-state.sh` also checks local `MTG` save slots for
populated decks below the 40-card minimum. The first rebuilt DLL used current
MinGW startup defaults that crashed during `DECKDLL.dll` `PROCESS_ATTACH` under
CrossOver; the accepted rebuild keeps the guard but disables dynamic base and
aliases `___ImageBase` to `__image_base__`, matching the legacy loader
assumptions.

Manual proof still requires replaying the witch/undead-knight encounter from
root `C:\Shandalar\Shandalar.exe` in CrossOver `MTG`, then recording whether the
duel reaches the first interactive turn without freezing before the coin flip.
