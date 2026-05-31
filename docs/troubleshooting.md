# Troubleshooting

## Assertion after choosing start color: `WM_CREATE CreateDIBSection`

### Symptoms

| Symptom | Notes |
| --- | --- |
| `Shandalar.exe` launches. | The executable starts before the failure. |
| New game starts. | The failure is after entering the new-game flow. |
| Choosing a starting color triggers an assertion. | Reported message includes `WM_CREATE CreateDIBSection`. |
| Wine/CrossOver may report a page fault. | Reported fault address was `0x00579fea` in `shandalar`. |

### Likely Causes

| Cause | Status |
| --- | --- |
| GDI/DIB graphics path failure after color selection. | Likely based on assertion and imports. |
| FaceMaker startup/resolution path failure during character creation. | Plausible, but no longer sufficient: user testing and the linked forum thread both say the no-resolution/Korath FaceMaker did not fix the assertion. |
| Too-small or missing Windows paging file in Wine/CrossOver. | Plausible; the same forum thread names the paging file, and the local `MTG` bottle had only `C:\pagefile.sys 27 77` before this pass. |
| Display mode, bit depth, palette, or scaling mismatch. | Plausible; needs testing. |
| Bad row/stride/buffer copy into a graphics surface. | Plausible from disassembly/registers. |
| Missing or malformed resource loaded after color selection. | Possible; needs file trace. |
| Wrong working directory or adjacent DLL layout. | Possible; current `MTG` evidence favors root `C:\Shandalar\Shandalar.exe`, while direct `C:\Shandalar\Program\Shandalar.exe` fails because `Program\zlib.dll` is missing. |
| Missing `D:\NewMagic\sources...` path. | Unproven and unlikely without runtime trace evidence. |

The `D:\NewMagic\sources...` part is probably a compile-time source path
embedded in the binary by an assertion macro. Do not create a fake `D:` drive
unless file tracing proves the game is trying to load runtime files there.

### Things to Try

| Try | Why |
| --- | --- |
| In CrossOver bottle `MTG`, launch `C:\Shandalar\Shandalar.exe` from `C:\Shandalar`. | This is the installed shortcut target and the logged path that opens root `Magic.exe`; it also has the root DLLs present. |
| On Windows or fresh Wine/CrossOver, record whether you launch from repo root or `Program/`. | Both layouts exist and differ in adjacent DLLs/assets; path matters. |
| Confirm `FaceMaker.exe` exists next to the active Shandalar launch path and has the documented `0x5f40` DIB patch. | This checkout now has active root and `Program` FaceMaker copies under the name Shandalar references; they are no-resolution/Korath-derived but no longer byte-identical to the reference `*-nores.exe` files. |
| Confirm `FaceData.txt`, `FaceButtons.txt`, and face art exist next to the active FaceMaker path. | FaceMaker uses these local support files during character creation. |
| Confirm `Window = 2` in both `Shandalar.ini` and `Program/Shandalar.ini`. | The shipped comments say mode 2 keeps Adventure Mode, Deckbuilder, and Facemaker in the windowed path and generally works better. |
| Avoid installing under `C:\Program Files`; use a writable folder such as `C:\Games\Shandalar`. | The forum thread flags permissions as a common trigger for this issue. |
| Keep a Windows paging file enabled and non-tiny. | The forum thread says this old graphics path can require swap space even on systems with plenty of RAM; the local `MTG` bottle was changed to `C:\pagefile.sys 512 1024`. |
| If duel prompts stop accepting `Done`, `Trigger`, or `Decline` in CrossOver bottle `MTG`, test app-default `Version=win7` with desktop `Shandalar1440` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`. | This keeps the user's preferred Win7 compatibility setting and tests a larger 4:3 virtual desktop after fullscreen was undesirable. Needs visible gameplay retest. |
| In CrossOver, test the fresh 32-bit `Shandalar-Win8-Test` bottle from `C:\Shandalar\Shandalar.exe`. | The existing `MTG` bottle is still Windows 7 at the system-registry level even with app-default `win8`; the fresh bottle reports Microsoft Windows 8 / `CurrentVersion=6.2` and now has a bottle-local `C:\Shandalar` copy. |
| Keep the existing `MTG` bottle as a comparison, not as the presumed fix. | User testing says FaceMaker/no-resolution plus `MTG` app-default `win8`, virtual desktop, pagefile, `Window = 2`, and the first Shandalar-only patch still reproduced the issue. Later active FaceMaker and Shandalar name-bypass patches have not yet had a successful full Shandalar-spawned visible retest. |
| In CrossOver, use a 32-bit XP bottle and a 32-bit Windows 7 bottle. | Compare old-Windows compatibility behavior if the fresh Win8 bottle also fails. |
| Enable virtual desktop at 640x480, 800x600, 1024x768, 1024x800, or 1440x1080. | Reduces display-size and fullscreen surprises. The current `MTG` retest uses `1440x1080` as a 4:3 step up for a 27-inch 5K display. |
| Disable high-DPI/Retina mode. | Old GDI code may not expect scaled surfaces. |
| Try 16-bit color or 256-color compatibility on native Windows. | Tests palette/DIB assumptions. |
| Test all five starting colors. | Separates color-specific resources from shared post-color screen failures. |
| Capture Wine/CrossOver logs with GDI/bitmap/file channels. | Finds missing files, DIB errors, and exact fault context. |
| Use the patched `Shandalar.exe` binaries in this checkout. | Root `Shandalar.exe` and `Program/Shandalar.exe` now pass `hSection = NULL` to `CreateDIBSection`; CrossOver smoke testing got past the reported crash point. |
| Use the combined-patch `Shandalar.exe` binaries in this checkout. | Root `Shandalar.exe` and `Program/Shandalar.exe` seed the name-entry buffer with `mPlayer` at file offset `0xa1a42`, bypass the fragile manual name editor at `0xa1acd`, and guard the final name copy through code cave `0x465170`/file offset `0x64570`. |
| Use the movement-stop patched `Shandalar.exe` binaries in this checkout. | Root `Shandalar.exe` and `Program/Shandalar.exe` include a same-arrow adventure-map stop code cave at `0x46502d`; visible gameplay testing still needs to confirm the exact feel. |
| Use the patched active `FaceMaker.exe` binaries in this checkout. | Root `FaceMaker.exe` and `Program/FaceMaker.exe` now pass `hSection = NULL` to their own `CreateDIBSection` wrapper; direct CrossOver FaceMaker startup rendered successfully. |

### What to Report

| Field | Example |
| --- | --- |
| Exact executable and path | `Program/Shandalar.exe` |
| Working directory | `Program/` |
| Platform | Native Windows, Wine, or CrossOver |
| Windows version reported to the game | XP, Windows 7, Windows 10, etc. |
| Bottle details | 32-bit vs 64-bit, installed runtimes/components |
| Display settings | Resolution, virtual desktop, high-DPI/Retina, color depth |
| Starting color | White, blue, black, red, or green |
| Exact assertion text | Include screenshot or text; verify `sldib` vs `sidlib` |
| Logs | Wine/CrossOver log or ProcMon CSV |

### Verified on this machine

| Check | Result |
| --- | --- |
| `cmp -s /private/tmp/FaceMaker-Korath-thread.exe FaceMaker-nores.exe` plus `find /Users/mdmoll/Shandalar -iname '*facemaker*korath*'` | The known downloaded thread helper matches the reference no-resolution copy. No repo file named `Facemaker-Korath.exe` was visible under `/Users/mdmoll/Shandalar` during this pass. |
| `shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe` | Active patched FaceMaker copies hash to `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`; reference no-resolution/Korath copies hash to `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`. |
| `cmp -l FaceMaker-nores.exe FaceMaker.exe` | Active FaceMaker differs from the no-resolution/Korath reference at 11 byte positions, matching the `CreateDIBSection hSection = NULL` patch. |
| `xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe Program/FaceMaker.exe` | Both active helpers begin `6a 00 57 50 8b 4d 10 51 ff 75 04` at the patch site. |
| `lldb` disassembly around `FaceMaker-Original.exe` / `FaceMaker-nores.exe` changed offsets | The Korath/no-resolution helper changes a resolution-argument branch, forces a 1024x768 fallback path, and stubs the `ChangeDisplaySettingsA` wrapper to return immediately. It does not remove `CreateDIBSection` or the GDI drawing path. |
| `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini` | Both repo files now use `Window = 2`. |
| `rg -n "^Window\\s*=" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.ini" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.ini"` | Both copied files inside the `MTG` bottle now use `Window = 2`. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" /v PagingFiles /t REG_MULTI_SZ /d "C:\pagefile.sys 512 1024" /f` | Completed successfully. |
| `rg -n "PagingFiles" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/system.reg"` | Shows `C:\pagefile.sys 512 1024`. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\Explorer\Desktops" /v Shandalar /t REG_SZ /d 1024x768 /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe" /v Version /t REG_SZ /d win8 /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe" /v Version /t REG_SZ /d win8 /f` | Completed successfully. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe" /v Version /t REG_SZ /d win8 /f` | Completed successfully. |
| `sed -n '795,825p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"` | Current duel-prompt retest state shows app-default `Version=win7` and desktop `Shandalar1440` entries for `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe`; `Shandalar1440` is `1440x1080`. |
| Timed logged launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | Stayed alive until the alarm; startup log showed `wined3d`, `explorer.exe /desktop`, 1024-wide 8bpp DIB sections, and no startup page fault. Visible start-color retest still needed. |
| Timed logged direct launch of `C:\Shandalar\FaceMaker.exe` in bottle `MTG` | Stayed alive until the alarm; startup log showed `wined3d`, `explorer.exe /desktop`, 1024x768 8bpp DIB sections, and no startup page fault. |
| Direct logged launch of patched `C:\Shandalar\FaceMaker.exe` in bottle `MTG` | Rendered FaceMaker UI; `/tmp/facemaker-direct-after-patch-cx.log` had no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| Visual smoke launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | Reached the `Magic: Shandalar` main menu. AppleScript, Wine SendKeys, and Swift/CoreGraphics attempts could focus or see the window but did not deliver a usable Start New Game click from this machine, so Start New Game still needs manual visual testing. |
| Direct logged launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` | Fails before gameplay due missing `Program\zlib.dll`, causing `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll` load failures. |
| Created CrossOver bottle `Shandalar-Win8-Test` | `cxbottle --bottle Shandalar-Win8-Test --create --template win8`; config reports `Template=win8`, `WineArch=win32`, registry reports `ProductName=Microsoft Windows 8`, `CurrentVersion=6.2`, and `CurrentBuild=9200`. |
| Timed logged launch of `Y:\Shandalar\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Stayed alive until manual cleanup after the timed smoke; `/tmp/shandalar-win8test-startup-cx.log` showed `wined3d` and many successful `NtGdiCreateDIBSection` calls, with no startup page fault. |
| Copied checkout into `Shandalar-Win8-Test` | Bottle-local path exists at `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar`, exposed as `C:\Shandalar`. |
| Set `ShandalarTall=1024x800` in `Shandalar-Win8-Test` | `user.reg` shows `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` using desktop `ShandalarTall`; `ShandalarTall` is `1024x800`. |
| Timed logged launch of `C:\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Stayed alive until manual cleanup after the timed smoke; `/tmp/shandalar-win8test-cdrive-tall-startup-cx.log` showed no startup page fault. |
| DIB binary patch applied to `Shandalar.exe` and `Program/Shandalar.exe` | File offset `0x1785b0` changed from `8b 55 04 51 57 50 8b 4d 10 51 52` to `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing `hSection = NULL`. The interim hSection-only hash was `73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`. |
| Name-entry binary patches applied to `Shandalar.exe` and `Program/Shandalar.exe` | File offset `0xa1a42` writes `mPlayer` into the buffer at `0x591228`; `0xa1acd` bypasses the fragile manual name editor by setting its return value to success; `0xa1af2` jumps to code at `0x465170`/file offset `0x64570` that writes `Player` if the accepted buffer is empty before copying to `0x7a0770`. The name-entry-only interim hash was `bd784cc248d08455270a6bfae5004ead8f9723d8017f8db152add113e8d3a9db`; the prior seed-plus-movement hash was `155a668c72867bd1274410eb05ca05fbb7bd9bed843b42d1583ea536805a4aaf`; both active files now hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`. |
| Same-arrow movement-stop binary patch applied to `Shandalar.exe` and `Program/Shandalar.exe` | Hooks at `0x44398c`, `0x444a2b`, and `0x444aa7` jump to code cave `0x46502d`; flag `0x583a2c` records that at least one movement step completed, then a repeated direction key can route to existing stop path `0x444a96`. |
| Bottle copies patched | `MTG` and `Shandalar-Win8-Test` copied `Shandalar.exe` files now hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; originals were backed up as `Shandalar.before-hsection-null-patch.exe` before the first DIB patch. |
| Patched `C:\Shandalar\Shandalar-nosection-test.exe` in `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the reported crash point; `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` shows `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` loading after DIB creation with no `Unhandled exception`, page fault, or assertion. |
| Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the reported crash point from the edited repo binary; `/tmp/shandalar-repo-patched-sendkeys-cx.log` shows the same post-color resources loaded from the repo path. |
| Patched `C:\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the reported crash point from the normal C-drive copied install; `/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` shows the same post-color resources loaded from the C-drive path. |
| Patched `C:\Shandalar\Shandalar.exe` in `MTG` plus Wine `wscript` SendKeys | Passed the reported crash point from the older practical shortcut path; `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log` shows the same post-color resources loaded from the C-drive path. |

See [docs/bugs/create-dibsection-after-color.md](bugs/create-dibsection-after-color.md)
for the full investigation.
