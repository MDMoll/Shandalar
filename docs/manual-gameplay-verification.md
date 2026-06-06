# Manual Gameplay Verification

This is the visible test plan needed before claiming that the cleaned branch
"works" end to end. Automated checks prove file identity and documentation
coverage; they do not prove gameplay.

For Codex/agent runtime attempts, first read
[runtime-testing-policy.md](runtime-testing-policy.md). Do not loop on
Wine/CrossOver GUI automation to fill this table; leave nondeterministic rows
for a human-visible pass.

Use exact paths. Record every result in this file, [running.md](running.md), or
a focused bug note. If a test fails, record the card/phase/window state before
changing settings.

Before a visible CrossOver test, run this from the repository root to print the
current branch, bottle settings, launch commands, and runtime hashes:

```sh
tools/print-manual-gameplay-baseline.sh
```

Paste that output into the result notes or use it to fill the environment table
below, then replace `Needs testing` cells with concrete pass/fail evidence.
For one-row updates without hand-editing Markdown tables, use:

```sh
tools/record-manual-gameplay-result.sh --field "Bottle Windows version" --value win7
tools/record-manual-gameplay-result.sh --test D2 --result "Fail: froze at post-combat Done; screenshot /path/to/screenshot.png"
```

The recorder only writes evidence text into this file. It does not launch the
game, infer pass/fail, or make incomplete evidence acceptable.

To summarize current gaps without launching the game, run:

```sh
tools/verify-manual-gameplay-results.sh --allow-incomplete
tools/verify-manual-gameplay-results.sh --allow-incomplete --show-missing
```

Before claiming the game works end to end, this command must pass without
`--allow-incomplete`:

```sh
tools/verify-manual-gameplay-results.sh
```

## Test Environment Record

The environment rows below are filled from the automated baseline generated on
2026-05-31 and refreshed for later runtime patch hashes on 2026-06-05. They
identify the current local `MTG` CrossOver test target and hashes, but they are
not visible gameplay results. The local `MTG` Manalink DLLs now include the
raw-mana snapshot, Piranha Marsh trigger-target, and Bojuka Bog trigger-target
patches, but this remains static copied-install evidence only.

| Field | Value |
| --- | --- |
| Date | 2026-05-31 19:52:00; runtime hashes refreshed 2026-06-05 |
| Tester | Codex visible macOS screenshot run with Wine SendKeys; later hash refresh was static verification, not a full human gameplay pass |
| Platform | macOS 15.7.5 arm64 via CrossOver |
| CrossOver or Wine version | CrossOver 26.1.0.39808 |
| Bottle name | MTG |
| Bottle Windows version | App-default `win7`; system registry Microsoft Windows 7 (6.1) |
| Virtual desktop | `Shandalar1440=1440x1080` |
| DuelOptions ShowCoinFlips | `0` |
| Working directory | `C:\Shandalar` |
| Command or shortcut target | `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"` |
| `Shandalar.exe` SHA-256 | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` |
| `Magic.exe` SHA-256 | `93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0` |
| `ManalinkEh.dll` SHA-256 | `c5e34db93b28bfc1552782f2035814cb847b9ca76d8dd7abe8b3770070bfa32e` |
| Repo `ManalinkEh.dll` SHA-256 | `c5e34db93b28bfc1552782f2035814cb847b9ca76d8dd7abe8b3770070bfa32e` |

## CrossOver `MTG` Baseline

Use the current local target unless deliberately testing another copy:

| Setting | Expected value |
| --- | --- |
| Bottle | `MTG` |
| Working directory | `C:\Shandalar` |
| Shandalar target | `C:\Shandalar\Shandalar.exe` |
| Root Magic target | `C:\Shandalar\Magic.exe` |
| Program Magic target | `C:\Shandalar\Program\Magic.exe` |
| App-default Windows version | `win7` |
| Virtual desktop | `Shandalar1440=1440x1080` |
| DuelOptions ShowCoinFlips | `0` |

Confirm hashes before testing:

```bat
certutil -hashfile C:\Shandalar\Shandalar.exe SHA256
certutil -hashfile C:\Shandalar\Magic.exe SHA256
certutil -hashfile C:\Shandalar\ManalinkEh.dll SHA256
```

Expected repo hashes are listed in [runtime-manifest.md](runtime-manifest.md).

## Core Shandalar Pass

| ID | Test | Expected result | Result |
| --- | --- | --- | --- |
| S1 | Launch root `Shandalar.exe` from the full checkout or `C:\Shandalar`. | Main menu appears. | Pass: visible launch of `C:\Shandalar\Shandalar.exe` in CrossOver `MTG` reached the `Magic: Shandalar` main menu; see [verified-on-this-machine.md](verified-on-this-machine.md) and [running.md](running.md). |
| S2 | Start a new game with white. | Character creation reaches the map with default `Player`; no `CreateDIBSection` assertion. | Pass: visible CrossOver `MTG` run launched `C:\Shandalar\Shandalar.exe`, Wine SendKeys selected Start New Game/default first color, and the game reached the adventure map with no visible `CreateDIBSection` assertion; cropped screenshot [generated/manual-gameplay/s2-map-2026-05-31.png](generated/manual-gameplay/s2-map-2026-05-31.png), notes [generated/manual-gameplay/s2-map-2026-05-31.md](generated/manual-gameplay/s2-map-2026-05-31.md), raw local log `/tmp/shandalar-visible-s2-attempt-cx.log`. |
| S3 | Repeat new-game start with blue. | Same as S2. | Needs testing |
| S4 | Repeat new-game start with black. | Same as S2. | Needs testing |
| S5 | Repeat new-game start with red. | Same as S2. | Needs testing |
| S6 | Repeat new-game start with green. | Same as S2. | Needs testing |
| S7 | On the adventure map, begin movement, wait at least one step, then press the same arrow again. | Movement stops through the existing stop path instead of continuing to collision. | Needs testing |
| S8 | Save the game, quit to menu, relaunch, and load the save. | Save/load works; save-state files are understood well enough to decide fixture/archive policy. | Needs testing |

## Duel Stability Pass

| ID | Test | Expected result | Result |
| --- | --- | --- | --- |
| D1 | Enter a duel from Shandalar and play at least five turns. | `Done`, `Trigger`, and `Decline` remain clickable. | Needs testing |
| D2 | Reach the post-combat main phase and click `Done`. | Phase advances; button does not freeze. | Needs testing |
| D3 | Trigger an optional creature or ability prompt and choose both available responses across repeated tests. | Prompt accepts input and resolves. | Needs testing |
| D4 | Test root `Magic.exe` directly from root or `C:\Shandalar`. | Duel shell opens or reports a visible, recorded dependency/config issue. | Needs testing |
| D5 | Test `Program\Magic.exe` directly from `Program\` or `C:\Shandalar\Program`. | Program Manalink path opens or reports a visible, recorded dependency/config issue. | Needs testing |

Related bounded runtime evidence: a 2026-06-04 CrossOver `MTG` log for
`C:\Shandalar\Shandalar.exe --e 0442 --p 0442` printed
`Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"`, opened root
`Magic.exe`, and showed no targeted coin-flip or fatal strings. The process was
killed after the alarm, so D1-D5 remain manual-visible tests.

## Regression Scenarios

| ID | Scenario | Expected result | Result |
| --- | --- | --- | --- |
| R1 | Opponent activates `Femeref Healer` after blockers are declared, targeting a creature that would otherwise die in combat. | Prompt/gameplay does not freeze; ability is only offered during the real damage-prevention window. | Needs testing |
| R2 | Repeat R1 with a similar Samite/Femeref/Kithkin prevention effect if available. | No input freeze. | Needs testing |
| R3 | During declare attackers, select an ordinary attacker, then click it again before `Done`. | It leaves the declared-attacker selection without becoming tapped or marked as having attacked. | Needs testing |
| R4 | Repeat R3 with multiple attackers. | Only the clicked attacker is removed; remaining attackers stay declared. | Needs testing |
| R5 | Repeat R3 with vigilance, attack-cost, attack-trigger, or banding edge cases if available. | Behavior is recorded; do not claim stable until edge cases are understood. | Needs testing |

## Result Template

| ID | Pass/fail | Exact path | Notes |
| --- | --- | --- | --- |
| Example | Needs testing | `C:\Shandalar\Shandalar.exe` | Record screenshot/log path if possible. |

## Failure Capture

When a freeze or crash happens, record:

| Field | What to capture |
| --- | --- |
| Visible state | Screenshot, active phase, active prompt/button, whose turn. |
| Card context | Card names, targets, combat state, stack/prompt text. |
| Path context | Exact `Shandalar.exe`, `Magic.exe`, and `ManalinkEh.dll` paths. |
| Runtime context | Bottle name, Windows version, virtual desktop, working directory. |
| Process evidence | CrossOver/Wine log, Windows Event Viewer note, or live process sample if available. |

Do not update [completion-audit.md](completion-audit.md) from "partially
proven" to "proven" until the relevant rows above have concrete pass/fail
evidence.
