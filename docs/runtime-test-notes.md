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
