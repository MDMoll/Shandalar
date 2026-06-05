# Piranha Marsh Spell Chain Freeze

## Symptom

The reported CrossOver `MTG` symptom was: an opponent played Piranha Marsh, the
duel stopped with the `Spell Chain` box open, the box could still be dragged,
and the rest of the duel did not respond.

This note records a targeted mitigation for one plausible cause. It is static
source/runtime evidence plus copied-install parity; it is not yet a visible
gameplay proof that the exact scenario is fixed.

## Finding

`card_piranha_marsh()` used a normal player-target prompt during its
enters-the-battlefield trigger:

```c
default_target_definition(player, card, &td, 0);
td.allow_cancel = 0;
td.zone = TARGET_ZONE_PLAYERS;
pick_target(&td, "TARGET_PLAYER");
```

That is a poor fit for an opponent-controlled automatic trigger. If the AI path
reaches a generic player prompt while the spell/trigger UI is still active, the
duel can look alive enough to drag the Spell Chain window while no game action
advances.

## Remediation

The source snapshots now route the trigger through `pick_player_duh(player,
card, 1-player, 0)`. In AI or Duh mode, that directly targets the opponent; in
ordinary human play, `pick_player_duh()` still falls back to the regular target
choice UI.

| Path | Change |
| --- | --- |
| `src/cards/zendikar.c` | Piranha Marsh ETB trigger uses `pick_player_duh()`. |
| `Program/src/cards/zendikar.c` | Mirrored source snapshot change. |
| `ManalinkEh.dll` | Runtime patch at file offset `0x3fe7a0`, VMA `0x023ff1a0`. |
| `Program/ManalinkEh.dll` | Runtime patch at file offset `0x3c4930`, VMA `0x023c5330`. |
| `tools/patch-piranha-marsh-trigger-target.py` | Guarded patch helper for repo and copied installs. |

The local `MTG` copied install was patched too, with backups preserved as
`ManalinkEh.before-piranha-marsh-target-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `74bd5a2ce59f17ef2f6bcdd267e9e42f55fc049086dd9ba5ca288f1e2ac99a3b` |
| `Program/ManalinkEh.dll` | `56e600222fd6d551667b8f256f671fb620ffe050c5d75e6fb67b962620364a7a` |

These hashes include the earlier damage-prevention, AI decision-time, and
raw-mana snapshot patches.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative byte checks:

```sh
xxd -p -l 75 -s $((0x3fe7a0)) ManalinkEh.dll
xxd -p -l 75 -s $((0x3c4930)) Program/ManalinkEh.dll
```

The root bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e881cd060085c00f84bfffffff89742404891c24e89dec0100c7442404010000008b4074890424e88aea0300e99bffffff
```

The Program bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e8c16a060085c00f84bfffffff89742404891c24e80dd60100c7442404010000008b4074890424e80aaf0300e99bffffff
```

Manual proof still requires replaying a duel where the opponent plays Piranha
Marsh and confirming that the trigger resolves and the duel remains interactive.
