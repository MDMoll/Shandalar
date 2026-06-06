# Duel Freeze: Damage-Prevention Activation

This note tracks the focused Femeref Healer freeze investigation. It separates
verified local facts from the current inference.

## Reported Repro

| Field | Detail |
| --- | --- |
| Platform | CrossOver on macOS, bottle `MTG`. |
| Symptom | Duel input stops responding after a few turns. |
| Buttons seen in related freezes | `Done`, `Trigger`, `Decline`; sometimes no dismiss button is visible. |
| Clearest card clue | Opponent used Femeref Healer after blockers were declared, preventing damage to the attacking creature. |
| Current CrossOver compatibility preference | Keep app-default `Version=win7`; do not switch the active `MTG` path to Win8 for this pass. |

## Verified Local Evidence

| Evidence | Result |
| --- | --- |
| `rg -n "Femeref Healer|Samite Healer|Kithkin Healer" magic_updater/ct_all.csv Program/magic_updater/ct_all.csv` | Femeref Healer, Samite Healer, and Kithkin Healer all use handler pointer `0x200c10e`. |
| `src/cards/unlimited.c:4090` and `Program/src/cards/unlimited.c:4033` | `card_samite_healer()` implements the shared handler. |
| Handler target setup | The handler uses `td.extra = damage_card` and `GAA_DAMAGE_PREVENTION`, so it targets internal pending-damage cards, not ordinary permanents. |
| Spell comparison | `GS_DAMAGE_PREVENTION` in `generic_spell()` already checks `LCBP_DAMAGE_PREVENTION` before offering damage prevention. |
| Activated path before patch | `card_samite_healer()` reached `generic_activated_ability()` without its own `LCBP_DAMAGE_PREVENTION` guard. |
| Activated handler inventory | `docs/generated/code-audit/damage-prevention-handlers.tsv` records 129 activated damage-prevention call-site rows across `src` and `Program/src`; 127 are now covered by the generic activated-ability helper guard in source and the 2 Samite/Femeref/Kithkin rows still show their same-function guard. |

## Inference

The likely freeze path is that the activated healer ability can be considered
outside the real damage-prevention window, such as during blocker declaration or
a stale prompt state. Since the target definition points at internal damage
cards, the engine can enter an input/prompt state that has no usable visible
choice, which matches the "buttons stop working" reports.

This is an inference from local source, card-table pointers, and the user
repro. It is not yet gameplay-proven.

## Activated Handler Inventory

The generated report
`docs/generated/code-audit/damage-prevention-handlers.tsv` is a static source
inventory produced by `tools/audit_codebase.py`. It looks for calls to the
generic activated-ability helpers that pass any `GAA_DAMAGE_PREVENTION*` flag.

| Metric | Result |
| --- | --- |
| Total call-site rows | 129 |
| `src` rows | 68 |
| `Program/src` rows | 61 |
| Rows with a same-function `LCBP_DAMAGE_PREVENTION` marker | 2 |
| Rows covered by `granted_generic_activated_ability()` | 127 |
| Rows still marked `needs-review` | 0 |
| Plain `GAA_DAMAGE_PREVENTION` rows | 117 |
| `GAA_DAMAGE_PREVENTION_CREATURE` rows | 8 |
| `GAA_DAMAGE_PREVENTION_ME` rows | 4 |

Limitations: the report is static source evidence, not a runtime repro list. A
single card can produce multiple rows, such as an `EVENT_CAN_ACTIVATE` probe and
the normal event path. `covered-by-generic-helper` means the current source
snapshot has a generic `LCBP_DAMAGE_PREVENTION` gate in
`granted_generic_activated_ability()`. The static report alone is not runtime
proof; the current runtime DLL hook/cave evidence is recorded below.

## Source Patches

Both source snapshots add the same card-specific guard at the top of
`card_samite_healer()` after the generic activated-ability event check:

```c
if (!(land_can_be_played & LCBP_DAMAGE_PREVENTION)){
	return 0;
}
```

Both source snapshots also add a generic activated-ability guard before target
selection in `granted_generic_activated_ability()`:

```c
if( (mode & (GAA_DAMAGE_PREVENTION | GAA_DAMAGE_PREVENTION_PLAYER | GAA_DAMAGE_PREVENTION_CREATURE | GAA_DAMAGE_PREVENTION_ME))
	&& !(land_can_be_played & LCBP_DAMAGE_PREVENTION)
  ){
	return 0;
}
```

Files changed:

| File | Purpose |
| --- | --- |
| `src/cards/unlimited.c` | Main source snapshot. |
| `Program/src/cards/unlimited.c` | Program-side source snapshot. |
| `src/functions/functions.c` | Main source snapshot generic activated-ability guard. |
| `Program/src/functions/functions.c` | Program-side source snapshot generic activated-ability guard. |

## Runtime Patch

The runtime fix is in `ManalinkEh.dll`, not `Shandalar.exe`.

| File | Patch site | New SHA-256 |
| --- | --- | --- |
| `ManalinkEh.dll` | Samite-family guard at file offset `0x3bb035`, function VMA `0x023bba20`; generic helper jump at `0x44cb23`; helper cave at `0x495a30` / VMA `0x02497030` | `63f03a0863b43c603b48d7ff20b9606dba247c27c0ae2f07a00cff237309fef1` |
| `Program/ManalinkEh.dll` | Samite-family guard at file offset `0x381a25`, function VMA `0x02382410`; generic helper jump at `0x40f115`; helper cave at `0x452c30` / VMA `0x02454030` | `70ae3f0ed9c76fea6cf715982a26882656a38d89467ec47ef93d3709f4ac1796` |

Expected bytes at both patch sites:

```text
f6 05 90 f1 4e 00 04 0f 84 ae 00 00 00 e9 01 00
```

Expected disassembly shape:

```asm
test byte ptr [0x4ef190], 0x4
je   existing_return_zero
jmp  normal_handler_body
nop
```

The broader generic helper guard is now also present in the runtime DLLs. The
`EVENT_CAN_ACTIVATE` branch in `_granted_generic_activated_ability` jumps to an
executable cave that:

```asm
test ebx, 0xf000000
je   original_target_check
test byte ptr [0x4ef190], 0x4
jne  original_target_check
xor  edx, edx
jmp  existing_return_zero
```

The helper cave then replays the original `GAA_CAN_TARGET` test before returning
to the original branch body. The generic helper patch originally bumped the
executable cave section virtual size from `0x30` to `0x80`; the later AI
raw-mana snapshot patch grows the same section to `0x100` in both DLLs so all
three current caves are mapped.

The same DLLs also contain the later AI decision-time clamp patch documented
in [opponent-turn-ai-decision-time.md](opponent-turn-ai-decision-time.md).
They also contain the AI raw-mana speculation snapshot patch documented in
[ai-raw-mana-snapshot.md](ai-raw-mana-snapshot.md).
Current hashes also include the later inline Piranha Marsh and Bojuka Bog
trigger-target patches plus the generic AI player-target selector patch
documented in
[ai-etb-player-target-spell-chain-freeze.md](ai-etb-player-target-spell-chain-freeze.md).

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
xxd -g1 -l 32 -s $((0x3bb035)) ManalinkEh.dll
xxd -g1 -l 32 -s $((0x381a25)) Program/ManalinkEh.dll
xxd -g1 -l 5 -s $((0x44cb23)) ManalinkEh.dll
xxd -g1 -l 38 -s $((0x495a30)) ManalinkEh.dll
xxd -g1 -l 5 -s $((0x40f115)) Program/ManalinkEh.dll
xxd -g1 -l 38 -s $((0x452c30)) Program/ManalinkEh.dll
objdump -Mintel -D --start-address=0x23bba20 --stop-address=0x23bba60 ManalinkEh.dll
objdump -Mintel -D --start-address=0x2382410 --stop-address=0x2382450 Program/ManalinkEh.dll
objdump -Mintel -D --start-address=0x2497030 --stop-address=0x2497058 ManalinkEh.dll
objdump -Mintel -D --start-address=0x2454030 --stop-address=0x2454058 Program/ManalinkEh.dll
```

## CrossOver Retest

Needs visible gameplay testing. The repo DLLs are patched, and the active local
`MTG` copied install at `C:\Shandalar` has been updated with matching DLLs.

| Active bottle path | Backup preserved before copy |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.before-femeref-healer-patch.dll`; `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.before-generic-damage-prevention-guard.dll` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.before-femeref-healer-patch.dll`; `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.before-generic-damage-prevention-guard.dll` |

Fully quit existing CrossOver Shandalar/Magic windows, relaunch root
`C:\Shandalar\Shandalar.exe` with working directory `C:\Shandalar`, and retest a
duel where Femeref Healer can activate after blockers are declared.

## Remaining Risk

| Risk | Next clue |
| --- | --- |
| A freeze may come from a different prompt family, not activated damage prevention. | Capture the exact card, phase, visible prompt/buttons, and whether CPU is busy/idle while frozen. |
| CrossOver event delivery is also involved. | If the freeze persists with the patched DLL copied into `MTG`, capture a process sample while the duel is frozen. |
| Root and `Program/` Manalink DLLs differ. | Keep testing paths explicit and record hashes. |
