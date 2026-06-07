# AI Player-Target Prompt And Spell Chain Freeze

## Symptom

The CrossOver `MTG` duel can stop with the `Spell Chain` box still movable when
an opponent-controlled land enters the battlefield and immediately targets a
player. The first reported case was Piranha Marsh; Bojuka Bog exposed the same
pattern.

A later manual test froze at the `Witch activates...` announcement for Augur of
Skulls. That card is not a land ETB trigger; it uses the generic activated
ability path with `TARGET_ZONE_PLAYERS`. That retest showed the problem class
also includes AI-controlled player-only activated abilities in Shandalar
adventure duels.

This note records static source/runtime evidence plus copied-install parity. It
is not visible gameplay proof that the Piranha Marsh or Bojuka Bog scenarios are
fixed.

## Manual Retest Result

Manual gameplay testing showed that Bojuka Bog was **not** fixed by the earlier
selector-side mitigation, was **not** fixed by the later preselection-only
mitigation, was **not** fixed by resolving the effect immediately during trigger
discovery, and was **not** fixed by suppressing the trigger during
`EVENT_TRIGGER` then resolving it during `EVENT_END_TRIGGER`. Later manual
testing also showed that simply routing the lands through the engine-native
`comes_into_play_mode(..., RESOLVE_TRIGGER_AI(player))` trigger path was still
not enough for Bojuka Bog. Later resolve-spell/sentinel, `duh_mode()`-guarded
resolve-trigger, and strict raw-AI card-callsite wrapper mitigations also failed
manual gameplay testing. A later Bojuka Bog manual retest still froze after the
ManalinkEh-only resolver stack-bypass patch; live-process inspection showed the
Shandalar adventure duel was loading `Shandalar.dll`, not `ManalinkEh.dll`, for
the relevant resolver/card code.

The current mitigation moves the fix one layer higher, into `resolve_trigger()`.
For non-speculating AI-owned lands whose trigger globals identify
`TRIGGER_COMES_INTO_PLAY`, the resolver bypasses `put_card_or_activation_onto_stack()`
and dispatches `EVENT_RESOLVE_TRIGGER` directly to the land before the Spell
Chain processing dialog can open. It then restores the trigger globals, clears
the processing/triggering flags it set, dispatches `EVENT_TRIGGER_RESOLVED`, and
returns from `resolve_trigger()`. Human, speculative, trace-mode, and non-land
triggers keep the original stack/dialog resolver path.

On 2026-06-07, Augur of Skulls froze manually after the Shandalar land-CIP
resolver patch, proving the previous fix was still too narrow. The follow-up
Shandalar DLL patch now also hooks `Target::real_select_target()` and skips the
old target-presentation selector for non-speculating AI pure player targets
after legal candidates have already been built. Fresh manual Augur, Piranha
Marsh, and Bojuka Bog retests are still required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. Augur of Skulls has an activated sacrifice ability that targets
a player during upkeep. Earlier mitigations changed the land target picker,
short-circuited the Manalink generic AI-only `TARGET_ZONE_PLAYERS` selection,
and wrapped the individual land `comes_into_play()` calls. Piranha Marsh still
froze manually after those card-specific wrapper attempts, so the land patch
treats the Spell Chain insertion itself as one failing layer.

The source snapshots now add an AI land CIP stack-bypass guard to
`resolve_trigger()`. The individual Piranha Marsh and Bojuka Bog bodies are back
to normal `comes_into_play(player, card, event) && pick_player_duh(...)`
resolution. The guard is constrained to `player == AI`, non-speculation,
`TRIGGER_COMES_INTO_PLAY`, matching trigger cause globals, and `TYPE_LAND`.

The runtime patch mirrors that source intent in both duel DLL families. The
ManalinkEh patch restores the Piranha Marsh and Bojuka Bog callsites to their
original `comes_into_play()` calls, then hooks `resolve_trigger()` before the
normal stack insertion. The Shandalar patch adds a small executable `.cdxai`
section to root and Program `Shandalar.dll`, hooks the adventure-duel
`resolve_trigger()` entry, and performs the same
AI/non-speculating/CIP/cause/land checks before directly resolving the land
trigger without opening Spell Chain. All non-matching contexts fall back to the
original resolver path.

The Augur failure showed that Shandalar adventure duels also need the generic
player-target selector hardening in `Shandalar.dll`, not only `ManalinkEh.dll`.
The follow-up patch hooks the C++ targeter at `0xcb16`, writes chosen candidate
index `0` only when `[this+0x8] == AI` and `[this+0x14] == TARGET_ZONE_PLAYERS`,
and otherwise calls the original selector. The guard is narrower than the whole
targeter: mixed creature/player targets, human targets, and candidate
construction keep the original path.

The source snapshots also harden the graveyard/exile helper used by Bojuka Bog:
they bound exile-zone scans, validate player/deck inputs, and clear only the
affected player's `graveyard_source` row. That helper hardening is source-only
until a future rebuilt DLL or dedicated helper patch exists; the shipped DLL
change in this pass is the resolver cave described above.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | ManalinkEh hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0`; Shandalar.dll C++ targeter hook/cave `0xcb16`/`0x1174920` in root and Program |
| AI land CIP resolver stack-bypass | `src/functions/events.c`; `Program/src/functions/events.c` | ManalinkEh resolver hook/cave `0x429acf`/`0x495b00`; Program `0x3ec7cf`/`0x452d00`; restored Piranha/Bojuka calls at `0x3fe77d`/`0x3f63bd` and Program `0x3c490d`/`0x3bc60d`; Shandalar.dll resolver hook `.cdxai` `0x94d34`/`0x1174800` in root and Program |
| Source-only exile helper hardening | `src/functions/deck.c`; `Program/src/functions/deck.c` | source snapshots only; no shipped DLL helper patch |

The local `MTG` copied install was patched too. Backups were preserved as
`ManalinkEh.before-ai-land-cip-stack-bypass-patch.dll` in the root and Program
install folders and as `Shandalar.before-ai-land-cip-stack-bypass-patch.dll` and
`Shandalar.before-ai-player-target-selection-patch.dll` beside the root and
Program Shandalar helper DLLs.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `Shandalar.dll` | `f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7` |
| `Program/Shandalar.dll` | `f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7` |
| `ManalinkEh.dll` | `68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5` |
| `Program/ManalinkEh.dll` | `619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI player-target selector, and AI
land CIP resolver stack-bypass handling patches. The Shandalar DLL hash is the
one used by the copied CrossOver `MTG` adventure-duel launch path, and includes
both the land-CIP resolver hook and the C++ targeter player-only hook.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
python3 tools/patch-ai-player-target-selection.py
python3 tools/patch-ai-etb-player-target-preselect.py
python3 tools/patch-ai-land-cip-trigger-stack-bypass.py --apply
tools/check-source-snapshot-parity.sh
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 Shandalar.dll Program/Shandalar.dll ManalinkEh.dll Program/ManalinkEh.dll
```

Representative AI land CIP resolver byte checks:

```sh
xxd -p -l 10 -s $((0x429acf)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3f63bd)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 247 -s $((0x495b00)) ManalinkEh.dll
xxd -p -l 10 -s $((0x3ec7cf)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3bc60d)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
xxd -p -l 247 -s $((0x452d00)) Program/ManalinkEh.dll
xxd -p -l 9 -s $((0x94d34)) Shandalar.dll
xxd -p -l 269 -s $((0x1174800)) Shandalar.dll
xxd -p -l 9 -s $((0x94d34)) Program/Shandalar.dll
xxd -p -l 269 -s $((0x1174800)) Program/Shandalar.dll
xxd -p -l 18 -s $((0xcb16)) Shandalar.dll
xxd -p -l 64 -s $((0x1174920)) Shandalar.dll
xxd -p -l 18 -s $((0xcb16)) Program/Shandalar.dll
xxd -p -l 64 -s $((0x1174920)) Program/Shandalar.dll
```

The root hook, restored calls, and cave bytes should be:

```text
e92ccc06009090909090
e8ceed0300
e80e6a0300
83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8d5c4f9ff83c40c85c07405e90f000000c705f8e9600000000000e98333f9ffc705f8e9600000000000ff35782d7a00ff35a4025902ff357cc16200ff35209a73005653e8f16cf8ff83c40889c7814f08000100106affba0100000029da526a7e5653e8122af9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3a40259028b44240ca3782d7a00816708fffeffef68000300005653e8d9e7f9fd83c40c8f05209a73008f057cc162008f05a40259028f05782d7a00e94734f9ff
```

The Program hook, restored calls, and cave bytes should be:

```text
e92c6f06009090909090
e81ead0300
e81e2a0300
83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8251afaff83c40c85c07405e90f000000c705f8e9600000000000e98390f9ffc705f8e9600000000000ff35782d7a00ff3524055402ff357cc16200ff35209a73005653e8f1e7f8ff83c40889c7814f08000100106affba0100000029da526a7e5653e89289f9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3240554028b44240ca3782d7a00816708fffeffef68000300005653e8d917fefd83c40c8f05209a73008f057cc162008f05240554028f05782d7a00e94791f9ff
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

The Shandalar player-target selector hook and cave bytes should be:

```text
e905fad40090909090909090909090909090
898d3cfdffff837f08017515817f1400100000750c31c0a36cd49400e9fe052bffb8f8424c00ffd0a16cd49400e9d6052bff0000000000000000000000000000
```

Manual proof still requires replaying duels where the opponent activates Augur
of Skulls and where opponents play the reported Piranha Marsh and Bojuka Bog
lands, then confirming the duel remains interactive. At least one additional
AI-controlled land ETB trigger and one additional AI player-target activated
ability should still be tested separately because these patches now cover both
the land-CIP resolver layer and the Shandalar player-target selector layer.
