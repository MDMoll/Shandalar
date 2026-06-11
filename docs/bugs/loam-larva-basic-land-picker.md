# Loam Larva Basic-Land Picker

## Symptom

During a Shandalar adventure duel, Loam Larva's enters-the-battlefield trigger
showed only the prompt bar `Select a basic land card.` and did not display a
usable library picker, even though the player's deck contained basic lands.

## Cause

The running process loaded `Shandalar.dll`, not `ManalinkEh.dll`. In that DLL,
Loam Larva is implemented by the generated C++ helper
`Card_card_etb_tutor_basic_land_to_top_optional::self()`, which builds a
`TYPE_LAND` target with `TARGET_SPECIAL_BASIC_LAND` and calls
`tutor_to_top_of_library()`.

The target path checks each library entry with
`card_not_in_play_has_subtype(iid, SUBTYPE_BASIC)`. Before this fix, the
`SUBTYPE_BASIC` case only searched the modern subtype list and missed the old
five-basic-land card-data family `SUB_BASIC_LAND` (`13`). That made old basic
Forest/Island/etc. cards fail the legal-candidate test before the deck dialog
could present a useful list.

## Fix

Root and `Program/Shandalar.dll` now share SHA-256:

`3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41`

The patch redirects the `SUBTYPE_BASIC` branch in
`card_not_in_play_has_subtype()` from file offset `0x7e395` into `.cdxai` cave
`0x1174960`. The cave accepts `BYTE [card_data + 0x29] == 0x0d` as basic, then
falls back to the original modern-subtype scan for non-basic-family records.

Backups are preserved as:

- `Shandalar.before-basic-land-subtype-patch.dll`
- `Program/Shandalar.before-basic-land-subtype-patch.dll`

The local CrossOver `MTG` copied install was updated with matching root and
Program DLLs, also preserving bottle-local backups. An already-running
Shandalar process can still have the old DLL mapped; restart before retesting.

## Verification

```sh
tools/verify-loam-larva-basic-land-picker.sh .
shasum -a 256 Shandalar.dll Program/Shandalar.dll
xxd -p -l 5 -s $((0x7e395)) Shandalar.dll
xxd -p -l 27 -s $((0x1174960)) Shandalar.dll
tools/verify-install-tree.sh .
tools/verify-crossover-mtg-state.sh
```

Expected hook/cave bytes:

- `0x7e395`: `e9c6e1cd00`
- `0x1174960`: `807e290d0f84f11c32ffbe01000000c745f001000000e9951c32ff`

Manual proof: on 2026-06-11, the user confirmed that after restarting
Shandalar, Loam Larva displayed selectable basic lands and resolved as expected
in an adventure duel.
