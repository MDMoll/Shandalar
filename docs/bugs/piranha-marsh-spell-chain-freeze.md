# Piranha Marsh Spell Chain Freeze

## Symptom

The reported CrossOver `MTG` symptom was: an opponent played Piranha Marsh, the
duel stopped with the `Spell Chain` box open, the box could still be dragged,
and the rest of the duel did not respond.

This note records targeted mitigations for plausible causes. It is static
source/runtime evidence plus copied-install parity; it is not yet a visible
gameplay proof that the exact scenario is fixed.

Bojuka Bog later exposed the same player-targeted ETB trigger pattern, and
manual Bojuka testing showed the selector-side and preselection-only mitigations
were not enough. See
[ai-etb-player-target-spell-chain-freeze.md](ai-etb-player-target-spell-chain-freeze.md)
for the broader investigation, failed trigger-time/trigger-mode mitigations, and
the newer AI land CIP resolver stack-bypass patch.

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

The source snapshots route the resolving trigger through `pick_player_duh(player,
card, 1-player, 0)`. In AI or Duh mode, that directly targets the opponent; in
ordinary human play, `pick_player_duh()` still falls back to the regular target
choice UI.

Later manual testing showed that card-callsite wrappers still were not enough.
The current mitigation restores Piranha Marsh's normal `comes_into_play()`
callsite and moves the shipped DLL fix into `resolve_trigger()`, before the
opponent's mandatory land ETB can be placed on Spell Chain.

| Path | Change |
| --- | --- |
| `src/cards/zendikar.c` | Piranha Marsh ETB trigger uses `pick_player_duh()`. |
| `Program/src/cards/zendikar.c` | Mirrored source snapshot change. |
| `ManalinkEh.dll` | Runtime patch at file offset `0x3fe7a0`, VMA `0x023ff1a0`. |
| `Program/ManalinkEh.dll` | Runtime patch at file offset `0x3c4930`, VMA `0x023c5330`. |
| `ManalinkEh.dll` | Piranha Marsh callsite restored at `0x3fe77d`; resolver hook/cave at `0x429acf`/`0x495b00`. |
| `Program/ManalinkEh.dll` | Piranha Marsh callsite restored at `0x3c490d`; resolver hook/cave at `0x3ec7cf`/`0x452d00`. |
| `tools/patch-piranha-marsh-trigger-target.py` | Guarded patch helper for repo and copied installs. |
| `tools/patch-ai-land-cip-trigger-stack-bypass.py` | Guarded resolver-level patch helper for repo and copied installs. |

The local `MTG` copied install was patched too, with backups preserved as
`ManalinkEh.before-piranha-marsh-target-patch.dll` and
`ManalinkEh.before-ai-land-cip-stack-bypass-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5` |
| `Program/ManalinkEh.dll` | `619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot patches, the later Bojuka Bog trigger-target patch, the generic AI
player-target selector patch, and the AI land CIP resolver stack-bypass patch.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-ai-land-cip-trigger-stack-bypass.py --apply
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative byte checks:

```sh
xxd -p -l 75 -s $((0x3fe7a0)) ManalinkEh.dll
xxd -p -l 75 -s $((0x3c4930)) Program/ManalinkEh.dll
xxd -p -l 10 -s $((0x429acf)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 10 -s $((0x3ec7cf)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
```

The root bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e881cd060085c00f84bfffffff89742404891c24e89dec0100c7442404010000008b4074890424e88aea0300e99bffffff
```

The Program bytes should be:

```text
c744240c00000000b80100000029d88944240889742404891c24e8c16a060085c00f84bfffffff89742404891c24e80dd60100c7442404010000008b4074890424e80aaf0300e99bffffff
```

The AI land CIP resolver hook and restored Piranha callsites should be:

```text
e92ccc06009090909090
e80e6a0300
e92c6f06009090909090
e81e2a0300
```

Manual proof still requires replaying a duel where the opponent plays Piranha
Marsh and confirming that the trigger resolves and the duel remains interactive.
