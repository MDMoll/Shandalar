# Runtime Test Notes

Runtime evidence is now sufficient to verify the patched binary gets past the
reported start-color crash point in CrossOver. It is still incomplete for full
gameplay because character naming, map control, save/load, and duels need a
visible/manual pass.

## Reported Reproduction

| Step | Reported result |
| --- | --- |
| Launch `Shandalar.exe` | Succeeds. |
| Start new game | Succeeds far enough to choose a starting color. |
| Choose starting color | Assertion dialog appears. |
| Continue/fail after assertion | Wine reports page fault in `shandalar` at `0x00579fea`. |

## Color Matrix

The automated pass used default/highlighted selections via Wine `wscript
SendKeys`, so it verifies the shared post-color path but not each color.

| Color | Result | Assertion? | Wine page fault? | Notes |
| --- | --- | --- | --- | --- |
| Default SendKeys path | Passed crash point | No assertion in log | No page fault in log | Loaded `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` after DIB creation. |
| White | Needs testing | Needs testing | Needs testing | Manual color-specific test in same bottle/settings. |
| Blue | Needs testing | Needs testing | Needs testing | Test in same bottle/settings. |
| Black | Needs testing | Needs testing | Needs testing | Test in same bottle/settings. |
| Red | Needs testing | Needs testing | Needs testing | Test in same bottle/settings. |
| Green | Needs testing | Needs testing | Needs testing | Test in same bottle/settings. |

## Working Directory Matrix

| Launch method | Result | Notes |
| --- | --- | --- |
| CrossOver `MTG`, `C:\Shandalar\Shandalar.exe`, workdir `C:\Shandalar` | Startup verified, start-color unverified | Timed log stayed alive; visual smoke reached the main menu under app-default desktop and `Version=win8`. |
| CrossOver `MTG`, `C:\Shandalar\FaceMaker.exe`, workdir `C:\Shandalar` | Startup verified, Shandalar-spawned path unverified | Direct timed log stayed alive and created 1024x768 8bpp DIB sections. |
| CrossOver `Shandalar-Win8-Test`, `Y:\Shandalar\Shandalar\Shandalar.exe`, workdir `Y:\Shandalar\Shandalar` | Startup verified, start-color unverified | Fresh 32-bit `win8` bottle; timed log stayed alive and showed successful DIB creation. |
| CrossOver `Shandalar-Win8-Test`, `C:\Shandalar\Shandalar.exe`, workdir `C:\Shandalar` | Startup verified, start-color unverified | Bottle-local install copy; app-default desktop `ShandalarTall=1024x800`; timed log stayed alive and showed no startup page fault. |
| CrossOver `Shandalar-Win8-Test`, patched `C:\Shandalar\Shandalar-nosection-test.exe`, workdir `C:\Shandalar` | Start-color crash point verified | Wine `wscript` SendKeys drove the flow far enough to load post-color resources with no original page fault/assertion. |
| CrossOver `Shandalar-Win8-Test`, patched repo `Y:\Shandalar\Shandalar\Shandalar.exe`, workdir `Y:\Shandalar\Shandalar` | Start-color crash point verified | This tested the actual patched repo `Shandalar.exe`; log showed post-color resource loads from the repo path. |
| CrossOver `Shandalar-Win8-Test`, patched `C:\Shandalar\Shandalar.exe`, workdir `C:\Shandalar` | Start-color crash point verified | This tested the normal patched bottle-local install path; log showed post-color resource loads from the C-drive path. |
| CrossOver `MTG`, patched `C:\Shandalar\Shandalar.exe`, workdir `C:\Shandalar` | Start-color crash point verified | This tested the older practical shortcut path after patching the copied install; log showed post-color resource loads from the C-drive path. |
| CrossOver `MTG`, `C:\Shandalar\Program\Shandalar.exe`, workdir `C:\Shandalar\Program` | Fails before gameplay | Missing `Program\zlib.dll` cascades into `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll`. |
| From `Program/` working directory on Windows/fresh Wine | Needs testing | Recommended comparison baseline only after adjacent DLLs/assets are confirmed. |
| From repo root on Windows/fresh Wine | Needs testing | Could expose relative-path assumptions. |
| Via `Manalink_Launcher.cmd` | Needs testing | More relevant to `Magic.exe` than the campaign bug. |
| Copy of only `Shandalar.exe` elsewhere | Not recommended | Would remove nearby runtime assets and DLLs. |

## Verified Local CrossOver Commands

```sh
/usr/bin/perl -e 'alarm 18; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\Shandalar.exe"

/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\FaceMaker.exe"

/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-win8test-startup-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "Y:\Shandalar\Shandalar\Shandalar.exe"

/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-tall-startup-cx.log --debugmsg +seh,+bitmap,+process "C:\Shandalar\Shandalar.exe"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-nosection-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar-nosection-test.exe"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-repo-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "Y:\Shandalar\Shandalar\Shandalar.exe"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"
```

The `MTG` commands exited by timed alarm rather than by an immediate exception.
The `Shandalar-Win8-Test` startup processes stayed alive past the timed wrapper
and were then killed manually. Relevant log evidence included `wined3d`,
`explorer.exe /desktop`, successful `NtGdiCreateDIBSection` calls, and no
startup page fault. The Shandalar visual smoke screenshot was captured at
`/tmp/shandalar-gui-smoke-2.png`.

The patched start-color smoke logs add stronger evidence:

| Log | Important matches |
| --- | --- |
| `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` | `NtGdiCreateDIBSection format (1024,-800)`, then `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`; no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| `/tmp/shandalar-repo-patched-sendkeys-cx.log` | Same post-color resources loaded from `Y:\Shandalar\Shandalar`; no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| `/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` | Same post-color resources loaded from `C:\Shandalar`; no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log` | Same post-color resources loaded from `C:\Shandalar` in the older `MTG` bottle; no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |

The patched repo command intentionally launched from `Y:` so the test exercised
the files edited in this checkout. The later `C:\Shandalar` command verifies
the local bottle copy after saving original backups and patching it in place.
