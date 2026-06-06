# Piranha Marsh Spell Chain Freeze

## Symptom

The reported CrossOver `MTG` symptom was: an opponent played Piranha Marsh, the
duel stopped with the `Spell Chain` box open, the box could still be dragged,
and the rest of the duel did not respond.

This note records targeted mitigations for plausible causes. It is static
source/runtime evidence plus copied-install parity; it is not yet a visible
gameplay proof that the exact scenario is fixed.

Bojuka Bog later exposed the same player-targeted ETB trigger pattern, and
manual Bojuka testing showed the selector-side mitigation was not enough. See
[ai-etb-player-target-spell-chain-freeze.md](ai-etb-player-target-spell-chain-freeze.md)
for the broader investigation and newer pre-priority target preselection patch.

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

A later mitigation also calls `ai_preselect_player_target_for_cip()` during
`EVENT_TRIGGER`, before the normal `comes_into_play()` path can expose an
unselected AI player-target trigger through Spell Chain priority.

| Path | Change |
| --- | --- |
| `src/cards/zendikar.c` | Piranha Marsh ETB trigger uses `pick_player_duh()`. |
| `Program/src/cards/zendikar.c` | Mirrored source snapshot change. |
| `src/functions/targets.c` | Shared `ai_preselect_player_target_for_cip()` helper. |
| `Program/src/functions/targets.c` | Mirrored source snapshot helper. |
| `ManalinkEh.dll` | Runtime patch at file offset `0x3fe7a0`, VMA `0x023ff1a0`. |
| `Program/ManalinkEh.dll` | Runtime patch at file offset `0x3c4930`, VMA `0x023c5330`. |
| `ManalinkEh.dll` | AI ETB preselection hook at `0x3fe77d`, cave `0x495b00`. |
| `Program/ManalinkEh.dll` | AI ETB preselection hook at `0x3c490d`, cave `0x452d00`. |
| `tools/patch-piranha-marsh-trigger-target.py` | Guarded patch helper for repo and copied installs. |
| `tools/patch-ai-etb-player-target-preselect.py` | Guarded patch helper for the pre-priority ETB target wrapper. |

The local `MTG` copied install was patched too, with backups preserved as
`ManalinkEh.before-piranha-marsh-target-patch.dll` and
`ManalinkEh.before-ai-etb-target-preselect-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `b9db52eacd267a81aed47977d6e43b935deda77b96bc431585ea093b5179fd4a` |
| `Program/ManalinkEh.dll` | `e51b36eb74ff46a760f8ba8af3c382d3344050ee9912511c9a12f92202f4d61f` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot patches, the later Bojuka Bog trigger-target patch, the generic AI
player-target selector patch, and the AI ETB player-target preselection patch.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-ai-etb-player-target-preselect.py
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative byte checks:

```sh
xxd -p -l 75 -s $((0x3fe7a0)) ManalinkEh.dll
xxd -p -l 75 -s $((0x3c4930)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
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

The AI ETB preselection Piranha hooks should be:

```text
e87e7f0900
e8eeed0800
```

Manual proof still requires replaying a duel where the opponent plays Piranha
Marsh and confirming that the trigger resolves and the duel remains interactive.
