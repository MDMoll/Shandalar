# Merfolk Sovereign AI Activation Freeze

## Symptom

On 2026-06-11, a Shandalar adventure duel froze when the AI activated Merfolk
Sovereign's tap ability:

`Tap: Target Merfolk creature can't be blocked this turn.`

The visible activation banner said `Merfolk Shaman activates...` while the card
shown was Merfolk Sovereign. Static deck/string evidence shows this label is the
Shandalar opponent/deck persona, not the card title:

- `decks/0150.dck` and `Program/decks/0150.dck` are titled `Merfolk Shaman`.
- The same deck contains four Merfolk Sovereign entries.
- `AdvStrings.txt`, `Menus.txt`, and their Program copies also contain
  `Merfolk Shaman`.

So the label is likely normal Shandalar UI wording. The freeze is the bug.

Screenshot evidence supplied by the user:

`/var/folders/d6/d72hmd1s5klcng3xpv0ms9g00000gp/T/codex-clipboard-4793020b-d43d-4de6-996e-0d290619df2e.png`

## Cause

`src/cards/m11.c` and `Program/src/cards/m11.c` implement Merfolk Sovereign with
`generic_activated_ability(..., GAA_UNTAPPED | GAA_CAN_TARGET |
GAA_LITERAL_PROMPT, ...)` and a `TYPE_CREATURE` target definition constrained to
`SUBTYPE_MERFOLK`.

In Shandalar adventure duels, `Shandalar.exe` loads `Shandalar.dll` for this
target path. The previous Shandalar C++ targeter patch bypassed the old
target-presentation selector only for AI pure player targets. Merfolk Sovereign
uses a pure in-play creature target, so it still reached the old selector path.

## Fix

Root and `Program/Shandalar.dll` now share SHA-256:

`3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41`

The existing Shandalar targeter hook at file offset `0xcb16` still jumps into
`.cdxai` cave `0x1174920`. The cave now writes chosen candidate index `0` when:

- `[this + 0x8] == AI`
- `[this + 0x14] == TARGET_ZONE_PLAYERS` or `TARGET_ZONE_IN_PLAY`

All human, mixed creature/player, speculative, and other target zones still call
the original selector. This preserves the legal candidate builder and avoids the
legacy selector for the Merfolk Sovereign class of non-speculating AI in-play
targets.

Backups are preserved as:

- `Shandalar.before-ai-in-play-target-selection-patch.dll`
- `Program/Shandalar.before-ai-in-play-target-selection-patch.dll`

The local CrossOver `MTG` copied install was patched too with matching backups.
An already-running Shandalar process can still have the old DLL mapped; fully
quit and relaunch before retesting.

## Verification

```sh
python3 tools/patch-ai-player-target-selection.py Shandalar.dll Program/Shandalar.dll
shasum -a 256 Shandalar.dll Program/Shandalar.dll
xxd -p -l 18 -s $((0xcb16)) Shandalar.dll
xxd -p -l 64 -s $((0x1174920)) Shandalar.dll
tools/verify-install-tree.sh .
tools/verify-crossover-mtg-state.sh
```

Expected hook/cave bytes:

- `0xcb16`: `e905fad40090909090909090909090909090`
- `0x1174920`: `898d3cfdffff837f0801751e817f14001000007409817f1400020000750c31c0a36cd49400e9f5052bffb8f8424c00ffd0a16cd49400e9cd052bff0000000000`

Static verification does not prove gameplay success. Manual proof still requires
restarting the `MTG` Shandalar process, replaying the Merfolk Shaman encounter,
letting the AI activate Merfolk Sovereign, and confirming the duel remains
interactive after the ability resolves.
