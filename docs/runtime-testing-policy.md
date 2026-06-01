# Runtime Testing Policy

This repo work is not primarily a Wine/CrossOver GUI-automation task.

Runtime testing is useful evidence, but it should not become the main work
loop. If GUI testing becomes unreliable, stop testing, document what happened,
and continue with repository work: documentation, source inspection, binary
inspection, inventory, cleanup planning, targeted crash analysis, and manual
test checklists.

## Core Rule

| Rule | Practical effect |
| --- | --- |
| Do not spend the bulk of a task driving the real game GUI through Wine, CrossOver, SendKeys, AppleScript, xdotool, or mouse-coordinate guesses. | Runtime testing is secondary evidence unless the user explicitly asks for a runtime-testing task. |
| Do not retry the same GUI path repeatedly. | One bounded attempt per target is enough unless the user asks for more. |
| Do not treat Wine/CrossOver behavior as native Windows behavior. | Keep native Windows instructions and CrossOver notes separate. |
| If focus, display mode, or keystrokes are nondeterministic, stop automation. | Record the failure and move back to repo work. |
| Prefer logs and static analysis over interactive menu exploration. | A single logged failure is usually more useful than a fragile playthrough. |

## Default Runtime Budget

Unless the user explicitly requests runtime testing, use at most one bounded
attempt for each relevant target:

| Target | Allowed bounded check |
| --- | --- |
| `Program/Shandalar.exe` | One launch attempt from its own directory or matching bottle path. |
| `Program/Magic.exe` | One launch attempt from `Program/` or matching bottle path. |
| `Shandalar.exe --help` | One capture attempt if command-line behavior is relevant. |
| Known crash path | One targeted reproduction attempt, such as start new game, choose starting color, and observe the `WM_CREATE CreateDIBSection` assertion/crash. |

After the attempt, stop runtime testing and write the result. Do not keep
relaunching in search of perfect GUI automation.

## SendKeys And GUI Automation

SendKeys-style automation is fragile. If used at all, it must be scripted,
short, deterministic, logged, documented, and stopped after the first clear
success or failure.

Acceptable uses:

| Use | Boundary |
| --- | --- |
| Verify that a known window appears. | Stop once the window opens or fails. |
| Reproduce a known crash once. | Stop after the crash or one failed reproduction attempt. |
| Advance through a known one-screen smoke path. | Do not use SendKeys to explore menus or gameplay. |

Do not build a large GUI automation framework unless the user explicitly asks
for that kind of task.

### Windows-Side SendKeys Under Wine

A tiny VBScript smoke check is acceptable for a narrow question:

```vbscript
Set sh = CreateObject("WScript.Shell")
sh.Run "C:\Shandalar\Shandalar.exe", 1, False
WScript.Sleep 3000
sh.AppActivate "Shandalar"
WScript.Sleep 500
sh.SendKeys "{ENTER}"
```

Example invocation:

```sh
wine wscript.exe Z:\\path\\to\\script.vbs
```

Use this to answer a small question like "did the game launch?" or "can the
known crash be reproduced once?" Do not use it for exploratory UI navigation.

### macOS/CrossOver Automation

AppleScript/System Events smoke checks are acceptable only when Accessibility
permissions are already configured and focus is deterministic:

```sh
osascript <<'APPLESCRIPT'
tell application "CrossOver" to activate
delay 1
tell application "System Events"
  key code 36 -- Return
  delay 0.5
  key code 53 -- Escape
end tell
APPLESCRIPT
```

Stop if the wrong window is focused, if the game changes display mode, or if
keystrokes are unreliable.

### X11 Automation

Use `xdotool` only in an X11 Wine environment where it is already known to
work. Do not assume it applies to macOS/CrossOver.

## Prefer Log-Based Evidence

For Wine/CrossOver, one logged run is often more useful than trying to drive
the GUI. A bounded Wine example:

```sh
cd /path/to/Shandalar/Program
mkdir -p ../docs/generated/runtime-logs
WINEDEBUG=+seh,+gdi,+bitmap,+loaddll,+file wine Shandalar.exe \
  > ../docs/generated/runtime-logs/shandalar-create-dibsection.log 2>&1
```

Then summarize the useful lines:

```sh
grep -iE "CreateDIBSection|DIB|bitmap|gdi|seh|exception|page fault|loaddll|NAME NOT FOUND|PATH NOT FOUND|D:\\\\NewMagic" \
  ../docs/generated/runtime-logs/shandalar-create-dibsection.log \
  > ../docs/generated/runtime-logs/shandalar-create-dibsection-summary.txt || true
```

For CrossOver, use the equivalent Run Command logging or `--cx-log` command
line once, then stop and document the result.

## Evidence To Record

When a runtime attempt is made, record:

| Field | Record |
| --- | --- |
| Date | Exact date of the attempt. |
| Executable | Exact path, such as `C:\Shandalar\Shandalar.exe`. |
| Working directory | Exact directory used to launch. |
| Command | Full command or CrossOver Run Command fields. |
| Runtime | Wine/CrossOver version, bottle/container name, Windows version, and display mode if known. |
| Visible result | Whether the app opened and whether the expected screen appeared. |
| Failure text | Exact crash/error text when available. |
| Evidence paths | Log path, screenshot path, and whether they are tracked or local-only. |
| Interpretation | Whether the behavior is Wine/CrossOver-specific, native Windows, or unknown. |

Use the narrowest appropriate place:

| Destination | Use |
| --- | --- |
| [runtime-test-notes.md](runtime-test-notes.md) | Short chronological notes for bounded runtime attempts. |
| [running.md](running.md) | Maintained launch commands, inventory, and test matrix. |
| [manual-gameplay-verification.md](manual-gameplay-verification.md) | Human-visible gameplay checklist results. |
| [bugs/](bugs/) | Focused crash or behavior investigations. |
| [generated/](generated/) | Preserved raw or generated evidence snapshots. |

## Known CreateDIBSection Crash Context

The current known start-color crash evidence includes:

```text
Assertion Error

File-> D:\NewMagic\sources\sldib\lib.c, Line-> 315
WM_CREATE CreateDIBSection
```

Wine Program Error Details included:

```text
Unhandled exception: page fault on write access to 0x0ffec000
in wow64 32-bit code (0x00579fea)

Backtrace:
=>0 0x00579fea in shandalar (+0x179fea) (0x000001e6)
0x00579fea shandalar+0x179fea: rep movsl (%esi), %es:(%edi)
```

Interpretation rules:

| Observation | Treat as |
| --- | --- |
| `D:\NewMagic\sources\sldib\lib.c` | Likely compile-time/debug path embedded in the binary unless runtime tracing proves otherwise. |
| `WM_CREATE CreateDIBSection` | The useful investigation target. |
| `rep movsl (%esi), %es:(%edi)` page fault | Likely downstream memory-copy failure after a bad or failed bitmap/DIB allocation. |

Do not assume the game is trying to read from `D:\NewMagic` without file-access
evidence.

## Preferred Static Investigation

Prefer static and targeted investigation before another GUI attempt:

1. Locate `sldib/lib.c` or equivalent source.
2. Search for `sldib`, `CreateDIBSection`, `WM_CREATE`, `DIBSection`,
   `Assertion Error`, `Line->`, and `File->`.
3. Identify which binary contains the assertion string.
4. Inspect imports for `GDI32.dll`, `CreateDIBSection`, `USER32.dll`,
   `DDRAW.dll`, and DirectDraw references.
5. Inspect bitmap parameters around the failing call: width, height, bit
   depth, palette, color table, device context, and allocation.
6. Identify assets loaded immediately after choosing a starting color.
7. Determine whether each starting color behaves differently through a manual
   checklist, not repeated exploratory automation.
8. Document native Windows test plans separately from Wine/CrossOver evidence.

Useful local command shapes:

```sh
rg -n -i "sldib|CreateDIBSection|WM_CREATE|DIBSection|Assertion Error|Line->|File->|CreateDIB" .
find . -iname 'lib.c' -o -iname '*dib*' -o -iname '*sldib*'
file Program/*.exe Program/*.dll 2>/dev/null || true
objdump -p Program/Shandalar.exe 2>/dev/null | rg -i "DLL Name|CreateDIBSection|GDI32|USER32|DDRAW|DirectDraw" -C 3
objdump -p Program/Magic.exe 2>/dev/null | rg -i "DLL Name|CreateDIBSection|GDI32|USER32|DDRAW|DirectDraw" -C 3
```

## Native Windows Testing Is Separate

Do not present Wine/CrossOver evidence as native Windows proof. For native
Windows testing, create a manual checklist and use tools such as:

| Tool | Use |
| --- | --- |
| Sysinternals Process Monitor | File, image-load, and registry tracing. |
| Dependencies.exe | Import/dependency inspection. |
| Dependency Walker | Legacy comparison only; expect false positives on modern Windows. |
| Event Viewer | Crash/error records when the app exits silently. |
| PowerShell `Get-FileHash` | Hash verification. |
| Visual Studio `dumpbin` | PE import inspection when available. |

Recommended Process Monitor filters:

```text
Process Name is Shandalar.exe
OR Process Name is Magic.exe

Operations:
- CreateFile
- Load Image
- RegOpenKey
- RegQueryValue
- RegSetValue

Results of interest:
- SUCCESS
- NAME NOT FOUND
- PATH NOT FOUND
- ACCESS DENIED
```

## Stop Conditions

Stop GUI testing immediately when:

| Stop when | Then do |
| --- | --- |
| The same crash/error is reproduced once. | Record the exact evidence and continue static/doc work. |
| The app does not receive SendKeys reliably. | Stop automation; do not try random key sequences. |
| The window cannot be focused deterministically. | Record that focus was nondeterministic. |
| The game changes display mode in a way that disrupts automation. | Stop and leave a human test step. |
| Automation requires guessing mouse coordinates. | Stop; mouse-coordinate guessing is not normal investigation work. |
| Wine/CrossOver behavior becomes the main bottleneck. | Resume repo-improvement work. |
| One useful log or screenshot has already been captured. | Preserve it and move on. |

SendKeys is a rubber mallet, not an archaeologist's brush.
