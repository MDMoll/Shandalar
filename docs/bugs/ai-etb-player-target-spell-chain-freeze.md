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
selector-side mitigations, was **not** fixed by the later preselection-only
mitigation, and was **not** fixed by resolving the effect immediately during
trigger discovery. Those mitigations remain present because they reduce the
generic AI player-target prompt surface, but they are no longer considered
sufficient for this bug.

The current mitigation stores a valid AI-controlled land ETB target during the
matching `EVENT_TRIGGER`, suppresses the Spell Chain stack item, and resolves
the existing small land effect during `EVENT_END_TRIGGER` after trigger dispatch
finishes. A fresh manual Bojuka Bog retest is still required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. The first mitigation routed their resolving code through
`pick_player_duh()`, and a second mitigation short-circuited the generic
AI-only `TARGET_ZONE_PLAYERS` selector. Bojuka still froze manually, so the
problem appears to occur before trigger resolution reaches that selector path.

The current source snapshots therefore call
`ai_preselect_player_target_for_cip()` immediately after
`comes_into_play_tapped()`. When that helper succeeds for non-speculating AI
during the matching `TRIGGER_COMES_INTO_PLAY` event, it preloads the opponent as
`targets[0]`, returns a suppress result during `EVENT_TRIGGER`, and returns a
resolve result during `EVENT_END_TRIGGER`. Piranha Marsh applies `lose_life()`
and Bojuka Bog applies `rfg_whole_graveyard()` only in that end-trigger resolve
case.

The runtime cave mirrors the important trigger identity checks and validates the
opponent. In the exact AI trigger context it stores `1-player` into
`targets[0]`, returns `0` during `EVENT_TRIGGER` so the trigger is not exposed
through Spell Chain, and returns `1` during `EVENT_END_TRIGGER` so the existing
Piranha Marsh or Bojuka Bog effect block runs after trigger dispatch. Otherwise
it calls the original `comes_into_play()` and preserves normal behavior. For
these land ETB triggers, the cave does not attempt to call every source helper
used by the C implementation; it is a targeted binary mitigation for the
observed shipped DLL layout.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0` |
| AI ETB player-target end-trigger resolution | same card files plus `targets.c`/`manalink.h` | Piranha/Bojuka hooks at `0x3fe77d`/`0x3f63bd`; cave `0x495b00`; Program hooks `0x3c490d`/`0x3bc60d`; cave `0x452d00` |

The local `MTG` copied install was patched too. New backups were preserved as
`ManalinkEh.before-ai-etb-target-end-trigger-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `7cb25032ee48c973b6e5ce17195607b5e3472ea457d60cc9421c320becadd927` |
| `Program/ManalinkEh.dll` | `f0a399ae7a8d65144f0d9bafff5a9287140c7c056fa0468d1cd9b5b07f3404d4` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI player-target selector, and AI
ETB player-target end-trigger-resolution patches.

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

Representative AI ETB end-trigger byte checks:

```sh
xxd -p -l 5 -s $((0x3f63bd)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 194 -s $((0x495b00)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3bc60d)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
xxd -p -l 194 -s $((0x452d00)) Program/ManalinkEh.dll
```

The root hook and cave bytes should be:

```text
e83e030a00
e87e7f0900
83ff7d0f840c00000081ff000b00000f85b100000083fb010f85a8000000833d74857200010f849b000000f60540067900020f858e000000813d782d7a00db0000000f857e000000391d848c73000f857200000039358c397a000f8566000000391d1c7e73000f855a000000391d7cc162000f854e0000003935209a73000f85420000005653e80543fdff83c40885c00f84300000005653e8d36cf8ff83c408ba0100000029da895074c74078ffffffffc640360181ff000b0000740331c0c3b801
```

The Program hook and cave bytes should be:

```text
e8ee700900
e8eeed0800
83ff7d0f840c00000081ff000b00000f85b100000083fb010f85a8000000833d74857200010f849b000000f60540067900020f858e000000813d782d7a00db0000000f857e000000391d848c73000f857200000039358c397a000f8566000000391d1c7e73000f855a000000391d7cc162000f854e0000003935209a73000f85420000005653e8d571fdff83c40885c00f84300000005653e8d3e7f8ff83c408ba0100000029da895074c74078ffffffffc640360181ff000b0000740331c0c3b801
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

Manual proof still requires replaying duels where opponents play the reported
Piranha Marsh and Bojuka Bog lands and confirming the duel remains interactive.
At least one additional mandatory AI-controlled ETB player-target trigger should
still be tested separately because this patch only bypasses Spell Chain for the
two known land call sites.
