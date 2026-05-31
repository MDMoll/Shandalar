# Local Helpers

This folder contains helper scripts used during local CrossOver/Wine testing.
They are not required for normal Windows play.

## Contents

| Path | Purpose |
| --- | --- |
| `crossover/inspect-shortcut.vbs` | Prints Windows shortcut target, arguments, and working directory when run under Wine/CrossOver Windows Script Host. |
| `crossover/sendkeys-start-color.vbs` | Sends a small sequence of Enter keys to try to advance the Shandalar start-color flow during a timed smoke test. |
| `crossover/sendkeys-to-name-entry.vbs` | Older SendKeys helper for reaching the name-entry part of new-game setup. |
| `crossover/sendkeys-to-name-entry-robust.vbs` | Longer-delay SendKeys helper for the same name-entry smoke path. |

## Policy

| Rule | Why |
| --- | --- |
| Record exact bottle, working directory, command, and log path when using these helpers. | GUI automation was partial and brittle in local testing. |
| Do not treat a SendKeys smoke as full gameplay verification. | It can prove a crash point was passed, but not that character creation, map movement, or duels are stable. |
| Keep these helpers separate from runtime folders. | They are local test aids, not game assets. |
