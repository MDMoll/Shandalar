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
selector-side mitigations, and was **not** fixed by the later preselection-only
mitigation. Those mitigations remain present because they reduce the generic AI
player-target prompt surface, but they are no longer considered sufficient for
this bug.

The current mitigation resolves the tiny AI-controlled land ETB effect during
the matching `EVENT_TRIGGER` and returns without setting `event_result`, so the
unresolved player-target trigger is not exposed through Spell Chain priority. A
fresh manual Bojuka Bog retest is still required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. The first mitigation routed their resolving code through
`pick_player_duh()`, and a second mitigation short-circuited the generic
AI-only `TARGET_ZONE_PLAYERS` selector. Bojuka still froze manually, so the
problem appears to occur before trigger resolution reaches that selector path.

The current source snapshots therefore call
`ai_preselect_player_target_for_cip()` immediately after
`comes_into_play_tapped()`. When that helper succeeds for non-speculating AI
during the matching `TRIGGER_COMES_INTO_PLAY` event, Piranha Marsh directly
applies `lose_life()` and Bojuka Bog directly applies `rfg_whole_graveyard()`,
then returns through `mana_producer()` without calling `comes_into_play()` for
that trigger.

The runtime cave mirrors the important trigger identity checks and validates the
opponent before distinguishing the Piranha Marsh and Bojuka Bog call sites by
their return addresses. In the exact AI trigger context it calls `lose_life()`
or `rfg_whole_graveyard()` immediately and returns `0`; otherwise it calls the
original `comes_into_play()` and preserves normal behavior. For these land ETB
triggers, the cave does not attempt to call every source helper used by the C
implementation; it is a targeted binary mitigation for the observed shipped DLL
layout.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0` |
| AI ETB player-target immediate resolution | same card files plus `targets.c`/`manalink.h` | Piranha/Bojuka hooks at `0x3fe77d`/`0x3f63bd`; cave `0x495b00`; Program hooks `0x3c490d`/`0x3bc60d`; cave `0x452d00` |

The local `MTG` copied install was patched too. New backups were preserved as
`ManalinkEh.before-ai-etb-target-immediate-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `98e067759a5c76f486e67c197d6522be570f717e449ae274e954a8fe99bf023f` |
| `Program/ManalinkEh.dll` | `86c733876b85029e489e69add6ff923322653670423214ccfcc544fc4ee871ba` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI player-target selector, and AI
ETB player-target immediate-resolution patches.

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

Representative AI ETB immediate-resolution byte checks:

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
83ff7d0f85b900000083fb010f85b0000000833d74857200010f84a3000000f60540067900020f8596000000813d782d7a00db0000000f8586000000391d848c73000f857a00000039358c397a000f856e000000391d1c7e73000f8562000000391d7cc162000f85560000003935209a73000f854a0000005653e81143fdff83c40885c00f8438000000b80100000029d8813c24c26d3f02740e813c2482f13f027411e91a00000050e83294f8ff83c40431c0c36a0150e8b46afaff83c40831c0c3
```

The Program hook and cave bytes should be:

```text
e8ee700900
e8eeed0800
83ff7d0f85b900000083fb010f85b0000000833d74857200010f84a3000000f60540067900020f8596000000813d782d7a00db0000000f8586000000391d848c73000f857a00000039358c397a000f856e000000391d1c7e73000f8562000000391d7cc162000f85560000003935209a73000f854a0000005653e8e171fdff83c40885c00f8438000000b80100000029d8813c2412d03b02740e813c2412533c027411e91a00000050e8a20bf9ff83c40431c0c36a0150e8c4c0faff83c40831c0c3
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

Manual proof still requires replaying duels where opponents play the reported
Piranha Marsh and Bojuka Bog lands and confirming the duel remains interactive.
At least one additional mandatory AI-controlled ETB player-target trigger should
still be tested separately because this patch only bypasses Spell Chain for the
two known land call sites.
