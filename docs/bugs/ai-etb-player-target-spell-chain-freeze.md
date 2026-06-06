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
selector-side mitigations. Those mitigations remain present because they reduce
the generic AI player-target prompt surface, but they are no longer considered
sufficient for this bug.

The newer mitigation preselects the AI-controlled ETB player target during
`EVENT_TRIGGER`, before the engine can expose the unresolved trigger through
Spell Chain priority. A fresh manual Bojuka Bog retest is still required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. The first mitigation routed their resolving code through
`pick_player_duh()`, and a second mitigation short-circuited the generic
AI-only `TARGET_ZONE_PLAYERS` selector. Bojuka still froze manually, so the
problem appears to occur before trigger resolution reaches that selector path.

The current source snapshots therefore call
`ai_preselect_player_target_for_cip()` immediately after
`comes_into_play_tapped()`. The helper only runs for non-speculating AI during
the matching `TRIGGER_COMES_INTO_PLAY` event for the affected card, then writes
the preferred player target before the normal `comes_into_play()` trigger path
continues.

The runtime cave mirrors the important trigger identity checks and validates the
opponent before writing `targets[0]`. For these land ETB triggers, the cave
does not attempt to call every source helper used by the C implementation; it is
a targeted binary mitigation for the observed shipped DLL layout.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0` |
| AI ETB player-target preselection | same card files plus `targets.c`/`manalink.h` | Piranha/Bojuka hooks at `0x3fe77d`/`0x3f63bd`; cave `0x495b00`; Program hooks `0x3c490d`/`0x3bc60d`; cave `0x452d00` |

The local `MTG` copied install was patched too. New backups were preserved as
`ManalinkEh.before-ai-etb-target-preselect-patch.dll` in the root and Program
install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `b9db52eacd267a81aed47977d6e43b935deda77b96bc431585ea093b5179fd4a` |
| `Program/ManalinkEh.dll` | `e51b36eb74ff46a760f8ba8af3c382d3344050ee9912511c9a12f92202f4d61f` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI player-target selector, and AI
ETB player-target preselection patches.

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

Representative AI ETB preselection byte checks:

```sh
xxd -p -l 5 -s $((0x3f63bd)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 137 -s $((0x495b00)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3bc60d)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
xxd -p -l 137 -s $((0x452d00)) Program/ManalinkEh.dll
```

The root hook and cave bytes should be:

```text
e83e030a00
e87e7f0900
83ff7d757883fb017573833d7485720001746af60540067900027561813d782d7a00db0000007555391d848c7300754d39358c397a007545391d1c7e7300753d391d7cc1620075353935209a7300752d5653e83943fdff83c40885c0741f5653e80b6df8ff83c408ba0100000029da895074c74078ffffffffc6403601575653e80beaf9ff83c40cc3
```

The Program hook and cave bytes should be:

```text
e8ee700900
e8eeed0800
83ff7d757883fb017573833d7485720001746af60540067900027561813d782d7a00db0000007555391d848c7300754d39358c397a007545391d1c7e7300753d391d7cc1620075353935209a7300752d5653e80972fdff83c40885c0741f5653e80be8f8ff83c408ba0100000029da895074c74078ffffffffc6403601575653e8ab3bfaff83c40cc3
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

Manual proof still requires replaying duels where opponents resolve the
reported Piranha Marsh and Bojuka Bog triggers, plus at least one additional
mandatory AI-controlled ETB player-target trigger, and confirming the duel
remains interactive.
