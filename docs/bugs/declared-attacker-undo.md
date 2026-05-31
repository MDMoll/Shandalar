# Declared Attacker Undo

## User-Facing Goal

| Goal | Status |
| --- | --- |
| Let the human player remove a creature from the declared-attackers selection before pressing Done. | Runtime patch added to root and `Program/` `Magic.exe`, and copied into local CrossOver bottle `MTG`; visible gameplay testing still needed. |

## Evidence

| Finding | Evidence |
| --- | --- |
| The attacker-selection UI is in `Magic.exe`, not the card DLLs. | `src/Magic-trace.c:3092` identifies `wndproc_AttackClass`; `src/Magic-trace.c:3100` identifies `wndproc_AttackSwordShield`. |
| The human attacker assignment path sets `STATE_ATTACKING` directly. | Static disassembly of `Magic.exe` shows `or byte [esi+0x8], 0x04` at `0x43c4f5`. |
| The pre-existing behavior rejects an already-selected attacker. | Before this patch, `Magic.exe` tested `[esi+0x8] & 0x20014` at `0x43c303` and jumped away when `STATE_ATTACKING` was present. |
| `remove_from_combat()` is not appropriate for this undo. | `src/functions/functions.c:13676` sets `STATE_ATTACKED` when clearing `STATE_ATTACKING`, which would incorrectly mark a pre-Done undo as a completed attack. |
| Attacker selection can already run cost/trigger hooks. | Static disassembly shows `TRIGGER_PAY_TO_ATTACK`-style handling before `STATE_ATTACKING` is set, and source notes at `src/defs.h:308` say human `TRIGGER_ATTACKER_CHOSEN` follows `PAY_TO_ATTACK`. |

## Patch

| File | Hook | Cave | Behavior |
| --- | --- | --- | --- |
| `Magic.exe` | `0x43c303` | `0x459bc8` | If the selected card already has `STATE_ATTACKING`, decrement the attacker count at `0x4ef170` when positive, clear only `STATE_ATTACKING`, reset `blocking` byte `0x24` to `-1`, and return to the normal picker loop. |
| `Program/Magic.exe` | `0x43c303` | `0x459bc8` | Same patch applied to the Manalink copy. |

The patch deliberately does not clear `STATE_TAPPED` because static evidence
shows normal attack tapping is handled later by `EVENT_DECLARE_ATTACKERS`, not
when the creature is first selected. It also avoids `STATE_ATTACKED`, so the
undo remains a pre-Done change of mind rather than a completed attack.

## Caveats

| Caveat | Why |
| --- | --- |
| Visible gameplay testing is still required. | This is a static binary patch; no automated CrossOver click path was able to drive combat selection. |
| Attack costs and attack-choice triggers may not be reversible. | The original assignment path can run hooks before and around attacker selection. This patch is safest for ordinary creatures with no attack taxes or special "when this attacks" costs. |
| Banding and unusual combat UI should be retested separately. | The assignment path manipulates a `blocking`/band byte at offset `0x24`; this patch resets it to `-1` like noncombat state. |

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`.

```sh
xxd -g1 -l 16 -s $((0x43c303-0x400000)) Magic.exe
xxd -g1 -l 80 -s $((0x459bc8-0x400000)) Magic.exe

xxd -g1 -l 16 -s $((0x43c303-0x400000)) Program/Magic.exe
xxd -g1 -l 80 -s $((0x459bc8-0x400000)) Program/Magic.exe
```

The hook should begin:

```text
e9 c0 d8 01 00 90 90 90 90 90 90 90 90
```

The cave should begin:

```text
f7 46 08 04 00 00 00 0f 85 12 00 00 00
```

## Verified on this machine

| Check | Result |
| --- | --- |
| `objdump -Mintel -D --start-address=0x459bc8 --stop-address=0x459c12 Magic.exe` and `Program/Magic.exe` | Both caves jump to `0x459be7` for `STATE_ATTACKING`, preserve the original rejection jump to `0x43c56a`, continue normal candidates at `0x43c310`, clear only `STATE_ATTACKING`, and return to `0x43c56a`. |
| `shasum -a 256 Magic.exe Program/Magic.exe` | Root patched hash is `5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1`; `Program/` patched hash is `0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa`. |
| Copied into bottle `MTG` | Active bottle files at `C:\Shandalar\Magic.exe` and `C:\Shandalar\Program\Magic.exe` now match the patched repo hashes. |
| Bottle backups | Previous bottle files were preserved as `C:\Shandalar\Magic.before-declared-attacker-undo-patch.exe` and `C:\Shandalar\Program\Magic.before-declared-attacker-undo-patch.exe`, with previous hashes `71609906df4d3e5f4aa004034b21cfa362fd2c5522d23c57c91d96d6ca7d7025` and `b36530d1244691b855df3ab6022b2587ddd0ec3b078b7389fb6e33f27fa482f0`. |
