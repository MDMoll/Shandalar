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

Manual proof still requires replaying the witch/undead-knight encounter from
root `C:\Shandalar\Shandalar.exe` in CrossOver `MTG`, then recording whether the
duel reaches the first interactive turn without freezing before the coin flip.
