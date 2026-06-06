# AI ETB Player-Target Spell Chain Freeze

## Symptom

The CrossOver `MTG` duel can stop with the `Spell Chain` box still movable when
an opponent-controlled land enters the battlefield and immediately targets a
player. The first reported case was Piranha Marsh; Bojuka Bog exposed the same
pattern.

This note records static source/runtime evidence plus copied-install parity. It
is not yet a visible gameplay proof that the exact Bojuka Bog scenario is fixed.

## Finding

Both reported lands had mandatory enters-the-battlefield triggers that targeted
a player through the generic selector. Piranha Marsh and Bojuka Bog used
card-local target definitions plus `pick_target(&td, "TARGET_PLAYER")` during
automatic trigger resolution.

The target engine has an AI selection branch, but it still enters the normal
target-selection machinery and side effects. For simple mandatory player-only
triggers, the existing safer helper is `pick_player_duh(player, card, 1-player,
0)`: AI and Duh mode target the opponent directly, while ordinary human play
still falls through to the regular target UI.

This was intentionally kept narrow. A global `pick_target()` rewrite would alter
ordinary AI decisions for many cards that target players, creatures, spells, or
mixed zones.

## Remediation

| Card | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | `ManalinkEh.dll` at `0x3fe7a0`; `Program/ManalinkEh.dll` at `0x3c4930` |
| Bojuka Bog | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | `ManalinkEh.dll` at `0x3f63e0`; `Program/ManalinkEh.dll` at `0x3bc630` |

The local `MTG` copied install was patched too, with backups preserved as
`ManalinkEh.before-bojuka-bog-target-patch.dll` in the root and Program install
folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `c5e34db93b28bfc1552782f2035814cb847b9ca76d8dd7abe8b3770070bfa32e` |
| `Program/ManalinkEh.dll` | `1de106b5f8d62cd7942c8da2086a60ba96932501f97fc363e0f51878ef4bdf47` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, and Bojuka Bog patches.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
tools/check-source-snapshot-parity.sh
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative Bojuka Bog byte checks:

```sh
xxd -p -l 117 -s $((0x3f63e0)) ManalinkEh.dll
xxd -p -l 117 -s $((0x3bc630)) Program/ManalinkEh.dll
```

The root bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e84151070085c00f84bfffffff89742404891c24e85d7002008b4074890424e8c2970200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090
```

The Program bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e8c1ed060085c00f84bfffffff89742404891c24e80d5902008b4074890424e8e27c0200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090
```

Manual proof still requires replaying a duel where the opponent plays Bojuka Bog
and confirming that the trigger resolves and the duel remains interactive.
