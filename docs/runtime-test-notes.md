# Runtime Test Notes

Use this file for short, chronological notes from bounded runtime attempts.
Long launch inventories belong in [running.md](running.md), full human gameplay
results belong in [manual-gameplay-verification.md](manual-gameplay-verification.md),
and raw/generated evidence belongs under [generated/](generated/).

Follow [runtime-testing-policy.md](runtime-testing-policy.md): one bounded
attempt per relevant target unless the user explicitly asks for more, then stop
and return to repository work if GUI automation is unreliable.

## Current Notes

| Date | Runtime | Result |
| --- | --- | --- |
| 2026-05-31 | CrossOver `MTG` | Root `C:\Shandalar\Shandalar.exe` reached the main menu and one default/first start-color path reached the adventure map. This is recorded as S1/S2 in [manual-gameplay-verification.md](manual-gameplay-verification.md). Remaining gameplay rows still need human-visible testing. |
| 2026-05-31 | CrossOver `MTG` | A later exploratory focus/SendKeys attempt became nondeterministic and was stopped. Per policy, do not keep iterating on GUI focus; use manual checklist rows or targeted one-shot logs instead. |
| 2026-06-01 | CrossOver `MTG` | Added byte-identical root `zlib.dll` to repo `Program/zlib.dll` and local `C:\Shandalar\Program\zlib.dll`. This resolves the previous missing-zlib dependency layout blocker for direct `Program/Shandalar.exe`; visible gameplay still needs manual testing. |
| 2026-06-01 | CrossOver `MTG` | Bounded direct launch of `C:\Shandalar\Program\Shandalar.exe` from `C:\Shandalar\Program` used `/usr/bin/perl -e 'alarm 25; exec @ARGV' ...`; it stayed alive until the alarm (`exit=142`) and `/tmp/shandalar-program-zlib-mtg-direct.log` was empty. The old immediate missing-`Program\zlib.dll`/`image.dll`/`DrawCardLib.dll`/`DECKDLL.dll` loader cascade was not reproduced, but no visible gameplay result was captured. |
