# Jungle Basin Generic Autotap Overpay

## Symptom

On 2026-06-11, a Shandalar adventure duel had Jungle Basin and Island on the
board. Casting Copper Myr, which costs `2`, autotapped both lands instead of
using only Jungle Basin. The result was one extra floating mana.

## Why It Matters

Mana burn is implemented in this engine. `mana_burn()` is called from the
turn/phase cleanup paths in `src/functions/engine.c` and
`Program/src/functions/engine.c`, so extra floating mana is potentially harmful
rather than only a display quirk.

## Cause

The Shandalar adventure-duel path loads `Shandalar.dll`. Its
`human_autotap_for_mana()` path tries lands in two passes:

1. relevant basic lands only, with `AUTOTAP_NO_NONBASIC_LANDS`
2. all lands

For a pure generic cost, the first pass can satisfy one point with a basic
Island before the all-land pass can use Jungle Basin. Jungle Basin then produces
two mana, leaving one extra mana floating after the `2` cost is paid.

## Fix

Root and `Program/Shandalar.dll` now share SHA-256:

`3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41`

The patch redirects the first-pass autotap flag write in `charge_mana_impl()`
from file offset `0x825c0` into `.cdxai` cave `0x1174980`.

The cave uses all-land first-pass flags `0x1c` only when the pending payment is
a simple pure generic cost of at least two. It preserves the original
basic-first flags `0x1e` for colored costs, flagged costs, and one-generic
costs.

The source mirrors in `src/functions/ai.c` and `Program/src/functions/ai.c`
have the same guard.

Backups are preserved as:

- `Shandalar.before-generic-autotap-order-patch.dll`
- `Program/Shandalar.before-generic-autotap-order-patch.dll`

The local CrossOver `MTG` copied install was updated with matching root and
Program DLLs, also preserving bottle-local backups. An already-running
Shandalar process can still have the old DLL mapped; fully quit and relaunch
before retesting.

## Verification

```sh
python3 tools/patch-shandalar-generic-autotap.py --dry-run Shandalar.dll Program/Shandalar.dll
shasum -a 256 Shandalar.dll Program/Shandalar.dll
xxd -p -l 7 -s $((0x825c0)) Shandalar.dll
xxd -p -l 82 -s $((0x1174980)) Shandalar.dll
tools/verify-install-tree.sh .
tools/verify-crossover-mtg-state.sh
make -B -C src WARNINGS_AS_ERRORS=1 functions/ai.o
make -B -C Program/src WARNINGS_AS_ERRORS=1 functions/ai.o
```

Expected hook/cave bytes:

- `0x825c0`: `e8bb9fcd009090`
- `0x1174980`: `50a190268e00a90000ffff753b6683f8027c35a194268e000b0598268e000b059c268e000b05a0268e000b05a4268e000b05a8268e000b05ac268e00750ac74424081c00000058c3c74424081e00000058c3`

Manual proof still needed: restart Shandalar, cast Copper Myr with Jungle Basin
and Island available, and confirm autotap uses only Jungle Basin.
