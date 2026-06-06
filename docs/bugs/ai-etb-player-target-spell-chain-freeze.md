# AI ETB Player-Target Spell Chain Freeze

## Symptom

The CrossOver `MTG` duel can stop with the `Spell Chain` box still movable when
an opponent-controlled land enters the battlefield and immediately targets a
player. The first reported case was Piranha Marsh; Bojuka Bog exposed the same
pattern.

This note records static source/runtime evidence plus copied-install parity. It
is not visible gameplay proof that the Bojuka Bog scenario is fixed.

## Manual Retest Result

Manual gameplay testing showed that Bojuka Bog was **not** fixed by the earlier
selector-side mitigation, was **not** fixed by the later preselection-only
mitigation, was **not** fixed by resolving the effect immediately during trigger
discovery, and was **not** fixed by suppressing the trigger during
`EVENT_TRIGGER` then resolving it during `EVENT_END_TRIGGER`.

The current mitigation no longer tries to hand-roll Spell Chain suppression.
Piranha Marsh and Bojuka Bog now use the engine-native
`comes_into_play_mode(player, card, event, RESOLVE_TRIGGER_AI(player))` path and
keep `pick_player_duh()` for the resolving player target. A fresh manual Bojuka
Bog retest is still required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. The first mitigation routed their resolving code through
`pick_player_duh()`, and a second mitigation short-circuited the generic
AI-only `TARGET_ZONE_PLAYERS` selector. Bojuka still froze manually, so the
problem appears to occur before trigger resolution reaches that selector path.

The source snapshots now call `comes_into_play_mode(...,
RESOLVE_TRIGGER_AI(player))` immediately after `comes_into_play_tapped()`.
That matches the normal convention used by many AI-owned trigger implementations:
AI/Duh mode resolves as mandatory, while non-Duh human control remains optional.
The resolving block still uses `pick_player_duh(player, card, 1-player, 0)` so
AI-controlled triggers target the opponent without opening a human target prompt.

The runtime cave mirrors that source intent. It computes `RESOLVE_TRIGGER_AI`
by calling `_duh_mode(player)`, then calls `_comes_into_play_mode(player, card,
event, mode)`. The existing Piranha Marsh and Bojuka Bog resolving blocks remain
responsible for `pick_player_duh()` and for applying `lose_life()` or
`rfg_whole_graveyard()`.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0` |
| AI ETB player-target trigger mode | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c`; `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | Piranha/Bojuka hooks at `0x3fe77d`/`0x3f63bd`; cave `0x495b00`; Program hooks `0x3c490d`/`0x3bc60d`; cave `0x452d00` |

The local `MTG` copied install was patched too. New backups were preserved as
`ManalinkEh.before-ai-etb-trigger-mode-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `752b6deb941cf75dfc846c023b78000766b7c79b7ef7c35505f5de830f08fd22` |
| `Program/ManalinkEh.dll` | `5dc6724b13b0ac3817561d4cad3e30a6f67fb6a2d45f2e259c715c46f08bcb9d` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI player-target selector, and AI
ETB player-target trigger-mode patches.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
python3 tools/patch-ai-player-target-selection.py
python3 tools/patch-ai-etb-player-target-preselect.py
tools/check-source-snapshot-parity.sh
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative AI ETB trigger-mode byte checks:

```sh
xxd -p -l 5 -s $((0x3f63bd)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 36 -s $((0x495b00)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3bc60d)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
xxd -p -l 36 -s $((0x452d00)) Program/ManalinkEh.dll
```

The root hook and cave bytes should be:

```text
e83e030a00
e87e7f0900
53e83a68f9ff83c40485c0b8010000007405b80200000050575653e8b0e9f9ff83c410c3
```

The Program hook and cave bytes should be:

```text
e8ee700900
e8eeed0800
53e89ac5f9ff83c40485c0b8010000007405b80200000050575653e8503bfaff83c410c3
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

Manual proof still requires replaying duels where opponents play the reported
Piranha Marsh and Bojuka Bog lands and confirming the duel remains interactive.
At least one additional mandatory AI-controlled ETB player-target trigger should
still be tested separately because this patch only changes the two known land
call sites.
