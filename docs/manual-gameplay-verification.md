# Manual Gameplay Verification

This is the visible test plan needed before claiming that the cleaned branch
"works" end to end. Automated checks prove file identity and documentation
coverage; they do not prove gameplay.

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
```

Before claiming the game works end to end, this command must pass without
`--allow-incomplete`:

```sh
tools/verify-manual-gameplay-results.sh
```

## Test Environment Record

| Field | Value |
| --- | --- |
| Date | Needs testing |
| Tester | Needs testing |
| Platform | Windows / CrossOver / Wine |
| CrossOver or Wine version | Needs testing |
| Bottle name | Needs testing |
| Bottle Windows version | Needs testing |
| Virtual desktop | Needs testing |
| Working directory | Needs testing |
| Command or shortcut target | Needs testing |
| `Shandalar.exe` SHA-256 | Needs testing |
| `Magic.exe` SHA-256 | Needs testing |
| `ManalinkEh.dll` SHA-256 | Needs testing |

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
| S1 | Launch root `Shandalar.exe` from the full checkout or `C:\Shandalar`. | Main menu appears. | Needs testing |
| S2 | Start a new game with white. | Character creation reaches the map with default `Player`; no `CreateDIBSection` assertion. | Needs testing |
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
