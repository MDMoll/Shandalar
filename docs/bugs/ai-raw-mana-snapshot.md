# AI Raw-Mana Snapshot During Speculation

This note tracks the `src/functions/ai.c` finding where AI speculation restored
both players' `raw_mana_available` rows from a stack snapshot that had only
saved one row.

## Finding

In `ai_decision_phase()`, the source and shipped DLLs temporarily replace the
opponent `raw_mana_available[p][0..7]` row with
`landsofcolor_controlled[p][0..7]` before `EVENT_SHOULD_AI_PLAY`. The old code
only initialized `store_raw_mana_available[1 - player][0..7]`, then restored
both player rows afterward.

That made the active player's row come back from uninitialized stack bytes
during AI speculation. The exact user-facing failure mode is unproven, but this
is a real state-corruption risk in a high-traffic opponent-turn path.

## Source Patch

Both source snapshots now save all 16 dwords before replacing only the opponent
row:

```c
for (p = 0; p <= 1; ++p)
	for (col = 0; col <= 7; ++col)
		store_raw_mana_available[p][col] = raw_mana_available[p][col];

p = 1 - player;
for (col = 0; col <= 7; ++col)
	raw_mana_available[p][col] = landsofcolor_controlled[p][col];
```

Files changed:

| File | Purpose |
| --- | --- |
| `src/functions/ai.c` | Main source snapshot. |
| `Program/src/functions/ai.c` | Program-side source snapshot. |

## Runtime Patch

The active repo DLLs now route the old snapshot block through an executable
cave that saves `raw_mana_available[0..1][0..7]`, then preserves the original
temporary opponent-row replacement.

| File | Patch site | Expected bytes | New SHA-256 |
| --- | --- | --- | --- |
| `ManalinkEh.dll` | Hook at file offset `0x40db84`, function VMA `0x0240e584`; cave at `0x495a90` / VMA `0x02497090` | Hook starts `e9 07 8b 08 00`; cave starts `8d 7d b4 31 c0 8b 0c 85 c0 f3 4e 00` and ends `e9 eb 74 f7 ff` | `cd9709398eba57d12044dcb936c2e728619a6eac3f401156b155efc6f872e656` |
| `Program/ManalinkEh.dll` | Hook at file offset `0x3d3844`, function VMA `0x023d4244`; cave at `0x452c90` / VMA `0x02454090` | Hook starts `e9 47 fe 07 00`; cave starts `8d 7d b4 31 c0 8b 0c 85 c0 f3 4e 00` and ends `e9 ab 01 f8 ff` | `6832a01eb11ae8872e4a00f8e8916e918a8538be865cf4bb43a9929cc690f07c` |

The shared executable cave section virtual size is now `0x100`, leaving the
existing generic damage-prevention cave at `+0x30`, the AI decision-time cave at
`+0x60`, and this raw-mana snapshot cave at `+0x90`.

## Patch Helper

`tools/patch-ai-raw-mana-snapshot.py` validates the exact old instruction
sequence, confirms the new cave is empty, and only then writes the hook, cave,
and section-size metadata.

Dry-run:

```sh
python3 tools/patch-ai-raw-mana-snapshot.py
```

Apply to the active repo DLLs:

```sh
python3 tools/patch-ai-raw-mana-snapshot.py --apply --backup-suffix .before-ai-raw-mana-snapshot-patch
```

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
xxd -g1 -l 5 -s $((0x40db84)) ManalinkEh.dll
xxd -g1 -l 59 -s $((0x495a90)) ManalinkEh.dll
xxd -g1 -l 5 -s $((0x3d3844)) Program/ManalinkEh.dll
xxd -g1 -l 59 -s $((0x452c90)) Program/ManalinkEh.dll
objdump -D -Mintel --start-address=0x0240e580 --stop-address=0x0240e5c0 ManalinkEh.dll
objdump -D -Mintel --start-address=0x02497090 --stop-address=0x024970d0 ManalinkEh.dll
objdump -D -Mintel --start-address=0x023d4240 --stop-address=0x023d4280 Program/ManalinkEh.dll
objdump -D -Mintel --start-address=0x02454090 --stop-address=0x024540d0 Program/ManalinkEh.dll
ALLOW_DIRTY=1 tools/verify-share-readiness.sh
```

## CrossOver State

The repo DLLs and local CrossOver `MTG` copied install were patched on
2026-06-05. Bottle backups were preserved beside the installed DLLs as
`ManalinkEh.before-ai-raw-mana-snapshot-patch.dll` and
`Program/ManalinkEh.before-ai-raw-mana-snapshot-patch.dll`.

## Remaining Risk

This is static/source and copied-runtime evidence only. It removes an observable
uninitialized restore pattern, but it does not prove any specific duel freeze
was caused by this bug. Visible opponent-turn gameplay still needs manual
testing before claiming the broader freeze class is fixed.
