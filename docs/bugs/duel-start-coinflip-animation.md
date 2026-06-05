# Duel-Start Coin-Flip Animation

This note tracks the focused mitigation for freezes that happen before the
starting coin flip or before the first turn starts.

## User-Facing Goal

| Goal | Status |
| --- | --- |
| Avoid a pre-duel freeze in the coin-flip animation/dialog path. | Root and `Program/` `Magic.exe` now default missing `ShowCoinFlips` registry data to off, and the local CrossOver `MTG` bottle explicitly sets `ShowCoinFlips=0`. A bounded direct-duel log opened root `Magic.exe` without targeted coin-flip/fatal strings; visible duel-start testing is still required. |

## Verified Local Evidence

| Evidence | Result |
| --- | --- |
| User-visible report | The game can freeze before even reaching the coin flip. |
| `Magic.exe` string scan | The binary contains `ShowCoinFlips`, `DIALOG_STARTCOINFLIP`, `DIALOG_COINFLIP`, `COINTOSS_Tails.AVI`, and `COINTOSS_Heads.AVI` strings. |
| Static disassembly around the registry fallback | When `ShowCoinFlips` is missing, both active `Magic.exe` copies previously wrote `1` to the global at `0x78725c`. |
| Static disassembly around `coin_flip()` | `coin_flip()` checks global `0x78725c`, then its third argument. If both are zero, it returns the random flip result without opening the coin-flip dialog. |
| Static disassembly around startup/mulligan call sites | Both active `Magic.exe` copies have `push 0` at file offsets `0x694b7` and `0x694eb` immediately before the two startup `coin_flip()` calls, matching the source comment that the final argument should be off during game startup. |
| Active `MTG` `user.reg` before this pass | `Software\\MicroProse\\Magic: The Gathering\\DuelOptions` had explicit `"ShowCoinFlips"="1"`, which would override a compiled default. |
| Active `MTG` `user.reg` after this pass | The same section has explicit `"ShowCoinFlips"="0"`. |
| Bounded direct-duel log after this pass | `C:\Shandalar\Shandalar.exe --e 0442 --p 0442` printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"`, opened root `Magic.exe`, and had no targeted `COINTOSS`, `ShowCoinFlips`, `DIALOG_*COINFLIP`, Hornet/card-data fatal, page fault, DIB assertion, or unhandled-exception strings. |

## Runtime Patch

The runtime patch changes only the missing-registry default. Existing registry
values still win, so the active `MTG` bottle also needed the explicit
`ShowCoinFlips=0` registry update.

| File | Patch site | Old bytes | Expected bytes | New SHA-256 |
| --- | --- | --- | --- | --- |
| `Magic.exe` | File offset `0x5db1f` | `c7 05 5c 72 78 00 01 00 00 00` | `c7 05 5c 72 78 00 00 00 00 00` | `93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0` |
| `Program/Magic.exe` | File offset `0x5db1f` | `c7 05 5c 72 78 00 01 00 00 00` | `c7 05 5c 72 78 00 00 00 00 00` | `685669692634ec830fe228904e11b1b536bd4b20e52192863a6280c2dbff6b66` |

## Startup Call-Site Guard

The startup/mulligan function at virtual address `0x469490` calls
`coin_flip()` twice. Both active `Magic.exe` copies pass `0` as the third
argument at file offsets `0x694b7` and `0x694eb`; that keeps the dialog
suppressed when `ShowCoinFlips=0`.

Expected byte prefixes:

| File offset | Expected bytes | Meaning |
| ---: | --- | --- |
| `0x694b7` | `6a 00 68 ac 5c 71 00 8b 45 10 50 e8 c9 71 00 00` | First startup-side `coin_flip()` call pushes `show_dialog_if_animation_is_off = 0`. |
| `0x694eb` | `6a 00 68 ac 5c 71 00 8b 45 10 50 e8 95 71 00 00` | Second startup-side `coin_flip()` call pushes `show_dialog_if_animation_is_off = 0`. |

Card coin flips still pass `1` through `player_flips_a_coin()`, so this guard is
specific to the duel-start path and does not globally remove card coin-flip
dialogs.

## Patch Helper

`tools/patch-magic-coinflip-default.py` validates the exact preimage bytes
before writing. It reports already-patched files as a no-op.

Dry-run:

```sh
uv run python tools/patch-magic-coinflip-default.py
```

Apply to the active repo executables with backups:

```sh
uv run python tools/patch-magic-coinflip-default.py --apply --backup-suffix .before-coinflip-default-patch
```

Apply to the local CrossOver `MTG` copied install with backups:

```sh
uv run python tools/patch-magic-coinflip-default.py --apply --backup-suffix .before-coinflip-default-patch "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Magic.exe" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Magic.exe"
```

## CrossOver State

The local `MTG` copied install was patched in place on 2026-06-04. Backups were
preserved before the write:

| Active bottle path | Backup preserved before patch |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Magic.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Magic.before-coinflip-default-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Magic.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Magic.before-coinflip-default-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.before-coinflip-default-patch.reg` |

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
uv run python tools/patch-magic-coinflip-default.py
shasum -a 256 Magic.exe Program/Magic.exe
xxd -g1 -l 10 -s $((0x5db1f)) Magic.exe
xxd -g1 -l 10 -s $((0x5db1f)) Program/Magic.exe
xxd -g1 -l 16 -s $((0x694b7)) Magic.exe
xxd -g1 -l 16 -s $((0x694eb)) Magic.exe
sed -n '260,280p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"
tools/verify-crossover-mtg-state.sh
ALLOW_DIRTY=1 tools/verify-share-readiness.sh
```

Expected `xxd` prefix for both `Magic.exe` copies:

```text
c7 05 5c 72 78 00 00 00 00 00
```

## Runtime Smoke

On 2026-06-04, a bounded CrossOver `MTG` command was run from repo root:

```sh
/usr/bin/perl -e 'alarm 25; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-root-ep0442-coinflip-default-cx.log --debugmsg +seh,+file,+process "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442
```

It printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"`, opened
root `C:\Shandalar\Magic.exe`, loaded AVI support, and opened
`statwin\water.avi`. Targeted log scans found no coin-flip AVI/dialog strings
and no app fatal/page-fault/assertion strings beyond missing Wine audio-driver
warnings. The child process stayed alive until the alarm and was killed
manually, so this is useful crash-path evidence, not visible gameplay proof.

## Remaining Risk

This is a narrow startup/UI mitigation. It does not prove the whole duel-start
path works, and it does not explain every possible freeze before the first
turn. If a duel still freezes, record whether the coin-flip animation appears,
whether the first-turn UI appears, the exact launch path, the active
`ShowCoinFlips` registry value, and whether CPU is busy or idle.
