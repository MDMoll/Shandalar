# Opponent-Turn AI Decision Time

This note tracks the focused hardening pass for opponent turns that appear to
freeze while the AI is speculating.

## Verified Local Evidence

| Evidence | Result |
| --- | --- |
| `src/functions/rules_engine.c` and `Program/src/functions/rules_engine.c` | The rules engine reads `config.txt`, not `Shandalar.ini`, for `AiDecisionTime`. |
| `config.txt` and `Program/config.txt` | Both active repo configs contain `AiDecisionTime:270`. |
| Local CrossOver `MTG` copied install | Both `C:\Shandalar\config.txt` and `C:\Shandalar\Program\config.txt` contain `AiDecisionTime:270`. |
| `src/functions/ai.c` and `Program/src/functions/ai.c` before this pass | If `AiDecisionTime` was missing or invalid, `_check_timer_for_ai_speculation` fell back to `5405`; high positive values were honored. |
| Root `ManalinkEh.dll` before this pass | `_check_timer_for_ai_speculation` contained `mov ebx, 0x151d` at file offset `0x40d0e7`. |
| `Program/ManalinkEh.dll` before this pass | `_check_timer_for_ai_speculation` contained `mov ebx, 0x151d` at file offset `0x3d2da7`. |
| Static clamp review after this pass | The current source snapshots and runtime DLLs clamp missing, invalid, or high positive `AiDecisionTime` values to `270`; configured values `1..270` are preserved. |

## Source Patch

Both source snapshots now use the configured fast value as the fallback and
upper bound:

```c
if (decision_time <= 0 || decision_time > 270){
	decision_time = 270;
}
```

Files changed:

| File | Purpose |
| --- | --- |
| `src/functions/ai.c` | Main source snapshot. |
| `Program/src/functions/ai.c` | Program-side source snapshot. |

## Runtime Patch

The active runtime fix is in `ManalinkEh.dll`, not `Magic.exe`. The earlier
fallback-immediate patch changed missing/invalid values from `0x151d` / 5405 to
`0x10e` / 270. The current patch goes further: it routes the decision-time setup
through a small executable cave so configured values `1..270` are preserved and
nonpositive or higher values use `270`.

| File | Patch site | Expected bytes | New SHA-256 |
| --- | --- | --- | --- |
| `ManalinkEh.dll` | Hook at file offset `0x40d0e1`, function VMA `0x0240dad0`; cave at `0x495a60` / VMA `0x02497060` | Hook `e9 7a 95 08 00 90 90 90 90 90 90`; cave `89 c3 85 c0 7e 08 81 fb 0e 01 00 00 7e 05 bb 0e 01 00 00 e9 74 6a f7 ff` | `cd9709398eba57d12044dcb936c2e728619a6eac3f401156b155efc6f872e656` |
| `Program/ManalinkEh.dll` | Hook at file offset `0x3d2da1`, function VMA `0x023d3790`; cave at `0x452c60` / VMA `0x02454060` | Hook `e9 ba 08 08 00 90 90 90 90 90 90`; cave `89 c3 85 c0 7e 08 81 fb 0e 01 00 00 7e 05 bb 0e 01 00 00 e9 34 f7 f7 ff` | `6832a01eb11ae8872e4a00f8e8916e918a8538be865cf4bb43a9929cc690f07c` |

Expected disassembly shape:

```asm
jmp  decision_time_cave
...
decision_time_cave:
mov  ebx, eax
test eax, eax
jle  set_default
cmp  ebx, 0x10e
jle  done
set_default:
mov  ebx, 0x10e
done:
jmp  original_time_division
```

The executable cave section virtual size is now `0x100` in both DLLs; the
preceding damage-prevention cave uses the same mapped section at offset `+0x30`,
this AI clamp cave uses offset `+0x60`, and the later raw-mana snapshot patch
uses offset `+0x90`.

## Patch Helper

`tools/patch-ai-decision-fallback.py` validates the older immediate-only patch.
`tools/patch-ai-decision-clamp.py` validates the exact setup preimage bytes,
the empty cave, and the executable section virtual size before writing. It
accepts the later `0x100` section size used by the raw-mana snapshot patch and
reports already-patched DLLs as a no-op.

Dry-run:

```sh
uv run python tools/patch-ai-decision-fallback.py
uv run python tools/patch-ai-decision-clamp.py
```

Apply to the active repo DLLs:

```sh
uv run python tools/patch-ai-decision-fallback.py --apply
uv run python tools/patch-ai-decision-clamp.py --apply --backup-suffix .before-ai-decision-clamp-patch
```

Apply to the local CrossOver `MTG` copied install with backups:

```sh
uv run python tools/patch-ai-decision-fallback.py --apply --backup-suffix .before-ai-decision-fallback-patch "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll"
uv run python tools/patch-ai-decision-clamp.py --apply --backup-suffix .before-ai-decision-clamp-patch "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll"
```

The same helper was run again with backup suffix `.before-ai-decision-270-patch`
when lowering the active fallback from 540 to 270.

The repo DLL hashes above include the later AI raw-mana snapshot patch. The
local CrossOver `MTG` copied install was also updated with that raw-mana patch
and now matches the repo Manalink hashes.

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
rg -n "^AiDecisionTime" config.txt Program/config.txt
shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll
xxd -g1 -l 11 -s $((0x40d0e1)) ManalinkEh.dll
xxd -g1 -l 24 -s $((0x495a60)) ManalinkEh.dll
xxd -g1 -l 11 -s $((0x3d2da1)) Program/ManalinkEh.dll
xxd -g1 -l 24 -s $((0x452c60)) Program/ManalinkEh.dll
objdump -D -Mintel --start-address=0x0240dad0 --stop-address=0x0240db10 ManalinkEh.dll
objdump -D -Mintel --start-address=0x02497060 --stop-address=0x02497090 ManalinkEh.dll
objdump -D -Mintel --start-address=0x023d3790 --stop-address=0x023d37d0 Program/ManalinkEh.dll
objdump -D -Mintel --start-address=0x02454060 --stop-address=0x02454090 Program/ManalinkEh.dll
ALLOW_DIRTY=1 tools/verify-share-readiness.sh
```

## CrossOver State

The local `MTG` copied install was patched in place on 2026-06-04 and
2026-06-05. Backups were preserved before the writes:

| Active bottle path | Backup preserved before patch |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.before-ai-decision-fallback-patch.dll` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.before-ai-decision-fallback-patch.dll` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/ManalinkEh.before-ai-decision-clamp-patch.dll` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.dll` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/ManalinkEh.before-ai-decision-clamp-patch.dll` |

## Remaining Risk

This is a bounded timer hardening patch. It does not prove that every
opponent-turn freeze is caused by overthinking, and it does not replace visible
gameplay testing. If a duel still freezes, record the exact card, phase,
visible buttons, whether CPU is busy or idle, and a process sample while the
window is frozen. Also record the exact `config.txt` path and active
`AiDecisionTime` value for the launch path being tested.
