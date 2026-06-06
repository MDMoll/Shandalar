# AI ETB Player-Target Spell Chain Freeze

## Symptom

The CrossOver `MTG` duel can stop with the `Spell Chain` box still movable when
an opponent-controlled land enters the battlefield and immediately targets a
player. The first reported case was Piranha Marsh; Bojuka Bog exposed the same
pattern.

This note records static source/runtime evidence plus copied-install parity. It
is not yet a visible gameplay proof that every affected ETB player-target
scenario is fixed.

## Finding

Both reported lands had mandatory enters-the-battlefield triggers that targeted
a player through the generic selector. Piranha Marsh and Bojuka Bog used
card-local target definitions plus `pick_target(&td, "TARGET_PLAYER")` during
automatic trigger resolution.

The deeper issue was not unique to those two card bodies. In
`select_target_impl()`, the AI branch already builds a candidate list and orders
player candidates according to `preferred_controller`, but non-speculating AI
selection still called `sub_499050()`, the generic target selector. That generic
selector path is the risky Spell Chain side-effect surface for automatic
AI-controlled player-only triggers.

A source scan found many mandatory ETB-style card functions that use
`TARGET_ZONE_PLAYERS` plus `pick_target()` or `TARGET_PLAYER`; examples include
Kessig Malcontents, Angel of Finality, Geralf's Mindcrusher, Sage's Row Denizen,
Balustrade Spy, Returned Centaur, Gatekeeper of Malakir, and Halimar Excavator.
Rather than inline-patching each card body, the shared selector now
short-circuits only the AI, non-speculating, pure `TARGET_ZONE_PLAYERS` case.
Human targeting, AI speculation, and mixed creature/player targets keep the
existing generic selector behavior.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | `ManalinkEh.dll` at `0x3fe7a0`; `Program/ManalinkEh.dll` at `0x3c4930` |
| Bojuka Bog | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | `ManalinkEh.dll` at `0x3f63e0`; `Program/ManalinkEh.dll` at `0x3bc630` |
| Generic AI player-only selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | hook/cave `0x469583`/`0x495ad0`; Program hook/cave `0x429453`/`0x452cd0` |

The local `MTG` copied install was patched too, with backups preserved as
`ManalinkEh.before-ai-player-target-selection-patch.dll` in the root and
Program install folders.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `ManalinkEh.dll` | `63f03a0863b43c603b48d7ff20b9606dba247c27c0ae2f07a00cff237309fef1` |
| `Program/ManalinkEh.dll` | `70ae3f0ed9c76fea6cf715982a26882656a38d89467ec47ef93d3709f4ac1796` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, and generic AI player-target selector
patches.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
python3 tools/patch-ai-player-target-selection.py
tools/check-source-snapshot-parity.sh
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
```

Representative generic selector byte checks:

```sh
xxd -p -l 17 -s $((0x469583)) ManalinkEh.dll
xxd -p -l 44 -s $((0x495ad0)) ManalinkEh.dll
xxd -p -l 17 -s $((0x429453)) Program/ManalinkEh.dll
xxd -p -l 44 -s $((0x452cd0)) Program/ManalinkEh.dll
```

The root hook and cave bytes should be:

```text
e948d10200909090909090909090909090
898d18fdffff817d1c00100000750d31d28915e42f7a00e9a82efdffe85f1f00fe8b15e42f7a00e9982efdff
```

The Program hook and cave bytes should be:

```text
e978a20200909090909090909090909090
898d18fdffff817d1c00100000750d31d28915e42f7a00e9785dfdffe85f4f04fe8b15e42f7a00e9685dfdff
```

Manual proof still requires replaying duels where opponents resolve the reported
Piranha Marsh and Bojuka Bog triggers, plus at least one additional mandatory
AI-controlled ETB player-target trigger, and confirming the duel remains
interactive.
