# Wine/CrossOver Debug Notes

The user-provided crash was from Wine/CrossOver. Follow-up local CrossOver
debug runs were performed after setting app-default virtual desktop and
`Version=win8` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`.

## Observed Environment From User Report

| Field | Value |
| --- | --- |
| Wine build | `wine-11.0-8720-g4351038808c` |
| Guest | `i386` |
| Reported Windows version | Windows 7 |
| Host | Darwin `24.6.0` |
| Fault module | `shandalar` |
| Fault address | `0x00579fea` |

The module list includes Wine GDI and graphics-adjacent modules such as
`gdi32`, `msimg32`, `gdiplus`, `msvfw32`, `winmm`, `winemac`, `user32`, and
`drawcardlib`.

## Suggested CrossOver/Wine Logging

From the executable directory:

```sh
WINEDEBUG=+seh,+gdi,+bitmap,+loaddll,+file wine Shandalar.exe > shandalar-create-dibsection.log 2>&1
```

For CrossOver:

1. Use Run Command or Run with Options.
2. Enable logging if available.
3. Record bottle name, CrossOver version, reported Windows version, 32-bit vs 64-bit bottle, installed components, and display settings.
4. For bottle `MTG`, launch root `C:\Shandalar\Shandalar.exe` from
   `C:\Shandalar`; direct `C:\Shandalar\Program\Shandalar.exe` currently fails
   earlier because `Program\zlib.dll` is absent.
5. Stop logging immediately after the assertion/page fault.

## Verified Local CrossOver Logs

| Log | Command summary | Result |
| --- | --- | --- |
| `/tmp/shandalar-mtg-win8-appdefault-cx.log` | Timed `C:\Shandalar\Shandalar.exe` from `C:\Shandalar` in bottle `MTG`. | Stayed alive until alarm; log showed `wined3d`, `explorer.exe /desktop`, 1024-wide 8bpp DIB sections, and no startup page fault. |
| `/tmp/facemaker-mtg-win8-appdefault-cx.log` | Timed direct `C:\Shandalar\FaceMaker.exe` from `C:\Shandalar` in bottle `MTG`. | Stayed alive until alarm; log showed `wined3d`, `explorer.exe /desktop`, 1024x768 8bpp DIB sections, and no startup page fault. |
| `/tmp/shandalar-win8test-startup-cx.log` | Timed `Y:\Shandalar\Shandalar\Shandalar.exe` from `Y:\Shandalar\Shandalar` in fresh bottle `Shandalar-Win8-Test`. | Stayed alive until cleanup; log showed `wined3d`, successful `NtGdiCreateDIBSection` calls, and no startup page fault. |
| `/tmp/shandalar-win8test-cdrive-startup-cx.log` | Timed `C:\Shandalar\Shandalar.exe` from `C:\Shandalar` in fresh bottle `Shandalar-Win8-Test` after copying the checkout into the bottle. | Stayed alive until cleanup; log used the C-drive install path and showed no startup page fault. |
| `/tmp/shandalar-win8test-cdrive-tall-startup-cx.log` | Timed `C:\Shandalar\Shandalar.exe` from `C:\Shandalar` in fresh bottle `Shandalar-Win8-Test` after setting `ShandalarTall=1024x800`. | Stayed alive until cleanup; log showed no startup page fault. |
| `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` | Patched bottle-local `C:\Shandalar\Shandalar-nosection-test.exe` plus Wine `wscript` SendKeys. | Passed the original crash point; log showed `NtGdiCreateDIBSection format (1024,-800)` followed by `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`; no `Unhandled exception`, page fault, or assertion. |
| `/tmp/shandalar-repo-patched-sendkeys-cx.log` | Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` plus Wine `wscript` SendKeys. | Passed the original crash point from the edited repo binary; same post-color resources loaded from the repo path; no `Unhandled exception`, page fault, or assertion. |
| `/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` | Patched bottle-local `C:\Shandalar\Shandalar.exe` plus Wine `wscript` SendKeys. | Passed the original crash point from the normal copied install path; same post-color resources loaded from the C-drive path; no `Unhandled exception`, page fault, or assertion. |
| `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log` | Patched `MTG` bottle-local `C:\Shandalar\Shandalar.exe` plus Wine `wscript` SendKeys. | Passed the original crash point from the older practical shortcut path; same post-color resources loaded from the C-drive path; no `Unhandled exception`, page fault, or assertion. |
| `/tmp/shandalar-win8test-cdrive-shandsave-cpath-magic3-cx.log` | Timed `/SHANDSAVE C:\Shandalar\MAGIC3.SVE` in the fresh bottle. | The explicit C-drive argument was preserved in the process command line, but the log did not prove that the save was directly opened or loaded. |

The SendKeys runs verify the reported start-color crash point only. They still
need a visible/manual gameplay pass before treating the patch as fully tested.

## Search Terms For Logs

```text
CreateDIBSection
WM_CREATE
GDI
bitmap
0x00579fea
0x00179fea
page fault
Shandalar.dll
Shandalar.exe
NAME NOT FOUND
```

## Wine-Specific Test Matrix

| Setting | Why |
| --- | --- |
| Fresh 32-bit Windows XP bottle | Older APIs/display assumptions may match better. |
| Fresh 32-bit Windows 7 bottle | Compare to the reported Windows 7 bottle. |
| Fresh 32-bit Windows 8 bottle | Compare to the linked forum thread's Windows 8 success clue. |
| Windows 10 bottle | Fallback/comparison after XP/7. |
| Virtual desktop at 640x480, 800x600, 1024x768, 1024x800 | Tests old display-size assumptions. `1024x800` is the current preferred manual test because startup DIB logs included 1024-wide and 800-high-ish surfaces. |
| Disable high-DPI/Retina mode | Avoids scaling surprises in old GDI code. |
| 16-bit or 256-color compatibility where available | Tests palette/bit-depth hypothesis. |
| Different renderer/backends only if CrossOver exposes them | Secondary evidence after logging exact failure. |
