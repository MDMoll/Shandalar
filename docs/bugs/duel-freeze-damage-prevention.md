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

## Inference

The likely freeze path is that the activated healer ability can be considered
outside the real damage-prevention window, such as during blocker declaration or
a stale prompt state. Since the target definition points at internal damage
cards, the engine can enter an input/prompt state that has no usable visible
choice, which matches the "buttons stop working" reports.

This is an inference from local source, card-table pointers, and the user
repro. It is not yet gameplay-proven.

## Source Patch

Both source snapshots now add the same guard at the top of
`card_samite_healer()` after the generic activated-ability event check:

```c
if (!(land_can_be_played & LCBP_DAMAGE_PREVENTION)){
	return 0;
}
```

Files changed:

| File | Purpose |
| --- | --- |
| `src/cards/unlimited.c` | Main source snapshot. |
| `Program/src/cards/unlimited.c` | Program-side source snapshot. |

## Runtime Patch

The runtime fix is in `ManalinkEh.dll`, not `Shandalar.exe`.

| File | Patch site | New SHA-256 |
| --- | --- | --- |
| `ManalinkEh.dll` | File offset `0x3bb035`, function VMA `0x023bba20` | `6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb` |
| `Program/ManalinkEh.dll` | File offset `0x381a25`, function VMA `0x02382410` | `7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517` |

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

Binary caveat: the C source keeps the original `IS_GAA_EVENT(event)` check and
then adds the damage-prevention-window guard. The in-place DLL patch gates the
handler at entry on `LCBP_DAMAGE_PREVENTION` and then continues to the existing
handler body. That is intentionally narrow for this healer family, but it still
needs visible gameplay testing.

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
xxd -g1 -l 32 -s $((0x3bb035)) ManalinkEh.dll
xxd -g1 -l 32 -s $((0x381a25)) Program/ManalinkEh.dll
objdump -Mintel -D --start-address=0x23bba20 --stop-address=0x23bba60 ManalinkEh.dll
objdump -Mintel -D --start-address=0x2382410 --stop-address=0x2382450 Program/ManalinkEh.dll
```

## CrossOver Retest

Needs visible gameplay testing. The repo DLLs are patched, and the active local
`MTG` copied install at `C:\Shandalar` has been updated with matching DLLs.

| Active bottle path | Backup preserved before copy |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.before-femeref-healer-patch.dll` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.before-femeref-healer-patch.dll` |

Fully quit existing CrossOver Shandalar/Magic windows, relaunch root
`C:\Shandalar\Shandalar.exe` with working directory `C:\Shandalar`, and retest a
duel where Femeref Healer can activate after blockers are declared.

## Remaining Risk

| Risk | Next clue |
| --- | --- |
| Another `GAA_DAMAGE_PREVENTION` handler can still be offered outside the window. | Capture the exact card and phase. |
| CrossOver event delivery is also involved. | If the freeze persists with the patched DLL copied into `MTG`, capture a process sample while the duel is frozen. |
| Root and `Program/` Manalink DLLs differ. | Keep testing paths explicit and record hashes. |
