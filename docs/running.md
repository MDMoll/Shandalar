# Running Shandalar and Magic.exe

Verified checkout path: `/Users/mdmoll/Shandalar/Shandalar`

## Launch Inventory

| Executable/script | Purpose | Working directory | Nearby files to preserve | Verified status | Notes |
| --- | --- | --- | --- | --- | --- |
| `Shandalar.exe` | Shandalar adventure shell and duel entry path. | repo root or `C:\Shandalar` in copied bottle | Root DLLs, `Shandalar.ini`, `Cards.dat`, `DBInfo.dat`, `Rarity.dat`, art/sound/resource folders, root `Magic.exe`. | Present, PE32 i386, and patched for the start-color `CreateDIBSection` crash plus default-name seed/bypass/fallback. In `Shandalar-Win8-Test`, the hSection patch passed the reported crash point; the later name-bypass patch still needs visible manual verification. | Same SHA-256 as `Program/Shandalar.exe`; local copied bottle installs were patched with backups. |
| `Magic.exe` | Duel executable opened by root `Shandalar.exe`. | repo root or `C:\Shandalar` in copied bottle | Root Manalink DLLs, card/art/deck/sound folders. | Present and PE32 i386. Opened by logged root Shandalar startup. Patched for declared-attacker undo; root `ManalinkEh.dll` is patched for the Samite/Femeref/Kithkin damage-prevention activation freeze. | Different SHA-256 from `Program/Magic.exe`; test separately. |
| `Program/Shandalar.exe` | Alternate Shandalar copy. | `Program/` | `Shandalar.dll`, `Shandalar.ini`, `Cards.dat`, `DBInfo.dat`, `Rarity.dat`, art/sound/resource folders, imported DLLs. | Present, PE32 i386, and patched the same way as root `Shandalar.exe`. Direct `MTG` launch fails before gameplay because `Program/zlib.dll` is absent. | Same SHA-256 as root `Shandalar.exe`, but the folder's adjacent DLL layout differs. |
| `Program/Magic.exe` | Manalink / launcher duel executable. | `Program/` | `ManalinkEh.dll`, `ManalinkEx.dll`, `Deckdll.dll`, `Drawcardlib.dll`, `CardArtLib.dll`, art/deck/sound folders. | Present and PE32 i386. Patched for declared-attacker undo; `Program/ManalinkEh.dll` has the same healer patch at its own offset. Earlier CrossOver run from `Program/` exited 53 with no visible result. | Treat as a first-class target. |
| `FaceMaker.exe` / `Program/FaceMaker.exe` | Character portrait/name helper used by Shandalar new-game setup. | Same folder as the active Shandalar launch | `FaceData.txt`, `FaceButtons.txt`, `FaceArt/` or `Faceart/`, `PlayFace/`, `Faces/` | Present and patched from the no-resolution/Korath helper; direct patched root FaceMaker launch in bottle `MTG` rendered without the DIB assertion. | Active root and `Program` copies no longer match `FaceMaker-nores.exe`; the `*-nores.exe` files remain as reference copies. |
| `Manalink_Launcher.cmd` | Windows batch menu for Manalink/mods. | repo root | `Program/`, `Mods/`, `PlayDeckAnalyser/` | Inspected only. | Lines 7-10 set `mlDir=Program`; lines 100-102 run `Magic.exe`. |
| `Shandalar help.bat` | Existing CLI example. | likely folder containing `Shandalar.exe` | `Shandalar.exe` and runtime assets. | Inspected only. | Contains `shandalar --e 0442 --p 0442`; meaning needs testing. |
| `archive/local-helpers/shandalar_homedoom.bat` | Local machine helper preserved from old root path `shandalar_homedoom.bat`. | hard-coded `e:\Program Files\Magic` | `RC.exe` | Stale/local; archived in limited reorg. | Not suitable as general launch documentation. |

## Native Windows Quick Start

1. Clone or download the whole repository. Do not copy only the `.exe` files.
2. Start with the root Shandalar executable from the full checkout:

```bat
cd \path\to\Shandalar
Shandalar.exe
```

3. If new-game setup fails after choosing a color, confirm this helper and its
   data exist:

```bat
dir FaceMaker.exe
dir FaceData.txt
dir FaceButtons.txt
dir FaceArt
```

4. Test root `Magic.exe` separately, then test the Manalink `Program` copy:

```bat
cd \path\to\Shandalar
Magic.exe

cd \path\to\Shandalar\Program
Magic.exe
```

5. If using the launcher menu, run from the repo root:

```bat
cd \path\to\Shandalar
Manalink_Launcher.cmd
```

## CrossOver on macOS Quick Start

Create a new 32-bit bottle first. For this checkout, a real Windows 8 test
bottle exists as `Shandalar-Win8-Test`. The patched `MTG` and
`Shandalar-Win8-Test` `C:\Shandalar\Shandalar.exe` launch paths have both
passed the reported start-color crash point.

The existing local `MTG` bottle has a copied install at `C:\Shandalar`. Its
shortcut targets root `C:\Shandalar\Shandalar.exe`. That copied executable was
patched in this pass, with a backup saved beside it. Direct
`C:\Shandalar\Program\Shandalar.exe` currently fails before gameplay in this
bottle because `Program\zlib.dll` is missing.

Backup note: bottle-local originals were saved as
`Shandalar.before-hsection-null-patch.exe` and
`FaceMaker.before-hsection-null-patch.exe` before patching.

Use CrossOver's Run Command for bottle `MTG`:

| Field | Value |
| --- | --- |
| Bottle | `MTG` |
| Command | `C:\Shandalar\Shandalar.exe` |
| Working directory | `C:\Shandalar` |
| Virtual desktop | `Shandalar1440`, `1440x1080` |
| App-default Windows version | `win7` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` |

Use CrossOver's Run Command for bottle `Shandalar-Win8-Test`:

| Field | Value |
| --- | --- |
| Bottle | `Shandalar-Win8-Test` |
| Command | `C:\Shandalar\Shandalar.exe` for the normal copied install, or `Y:\Shandalar\Shandalar\Shandalar.exe` for the repo path |
| Working directory | `Y:\Shandalar\Shandalar` or `C:\Shandalar` to match the command |
| Virtual desktop | `ShandalarTall`, `1024x800` |
| Paging file | `C:\pagefile.sys 512 1024` |

This bottle was created from CrossOver's 32-bit `win8` template. Local registry
evidence reports `ProductName=Microsoft Windows 8`, `CurrentVersion=6.2`, and
`CurrentBuild=9200`. The checkout was then copied into the bottle at
`/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar`
so the next test uses a normal `C:` install path instead of the macOS `Y:`
drive mapping.

Then repeat with:

```text
C:\Shandalar\Magic.exe
C:\Shandalar\Program\Magic.exe
```

Command-line equivalent if CrossOver's `wine` helper is available:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" "Y:\Shandalar\Shandalar\Shandalar.exe"
```

For a fresh bottle that launches directly from the macOS checkout, start with:

```sh
cd /Users/mdmoll/Shandalar/Shandalar
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME Shandalar.exe
```

## Wine Quick Start

Standalone Wine was not found in `PATH` on this machine. If it is installed on
another machine, use:

```sh
cd /path/to/repo
wine Shandalar.exe --help
wine Shandalar.exe

cd /path/to/repo/Program
wine Magic.exe
```

## Verified on this machine

| Command | Working directory | Result |
| --- | --- | --- |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --version` | repo root | CrossOver 26.1.0.39808. |
| `command -v wine` | repo root | No standalone Wine found. |
| `shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe` | repo root | Active FaceMaker copies hash to `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`; no-resolution/Korath reference copies hash to `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`. |
| `xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe Program/FaceMaker.exe` | repo root | Both patched dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing FaceMaker's `CreateDIBSection` wrapper to receive `hSection = NULL`. |
| `cmp -s Program/FaceData.txt Manalink3/Program/FaceData.txt` | repo root | Match; FaceMaker data is present in `Program/`. |
| `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini` | repo root | Both active configs should show `Window = 2` for the current CrossOver start-color test. |
| `diff -qr Program/FaceArt Manalink3/Program/FaceArt` | repo root | No differences. |
| `shasum -a 256 Shandalar.exe Program/Shandalar.exe` | repo root | Match; root and `Program/` Shandalar binaries hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`. |
| `xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing `CreateDIBSection` to receive `hSection = NULL`. |
| `xxd -g1 -l 40 -s $((0xa1a42)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `c7 05 28 12 59 00 6d 50 6c 61`, seeding the name buffer with `mPlayer` before the existing gender/name handling code runs. |
| `xxd -g1 -l 32 -s $((0xa1acd)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `31 c0 89 85 a8 fe ff ff`, bypassing the fragile manual name editor. |
| `xxd -g1 -l 64 -s $((0x64570)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps contain the empty-name fallback that writes `Player` before copying to `0x7a0770`. |
| Focused `lldb` checks for `0x44398c`, `0x444a2b`, `0x444aa7`, and `0x46502d` | repo root | Static verification shows movement hooks at `0x44398c`, `0x444a2b`, and `0x444aa7`, with code cave `0x46502d` and flag `0x583a2c` for same-arrow stop behavior. GNU `objdump -D -Mintel` is also usable when installed. |
| `shasum -a 256 Magic.exe Program/Magic.exe` | repo root | Differ; test both exact paths. |
| `xxd -g1 -l 16 -s $((0x43c303-0x400000)) Magic.exe` and the same command for `Program/Magic.exe` | repo root | Both begin `e9 c0 d8 01 00 90 90 90 90 90 90 90 90` after the declared-attacker undo patch. |
| `xxd -g1 -l 80 -s $((0x459bc8-0x400000)) Magic.exe` and the same command for `Program/Magic.exe` | repo root | Both caves begin `f7 46 08 04 00 00 00 0f 85 12 00 00 00`. |
| `shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll` | repo root | Root patched hash is `6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb`; `Program/` patched hash is `7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517`. |
| `xxd -g1 -l 32 -s $((0x3bb035)) ManalinkEh.dll` and `xxd -g1 -l 32 -s $((0x381a25)) Program/ManalinkEh.dll` | repo root | Both patched dumps begin `f6 05 90 f1 4e 00 04 0f 84 ae 00 00 00 e9 01 00`, gating the shared Samite/Femeref/Kithkin healer handler on `LCBP_DAMAGE_PREVENTION`. |
| `/usr/bin/perl -e 'alarm 18; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\Shandalar.exe"` | repo root | Stayed alive until the alarm; log showed app-default virtual desktop/`Version=win8`, `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\FaceMaker.exe"` | repo root | Stayed alive until the alarm; log showed app-default virtual desktop/`Version=win8`, `wined3d`, `explorer.exe /desktop`, and 1024x768 8bpp DIB sections. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-direct-after-patch-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\FaceMaker.exe"` | repo root | Direct patched FaceMaker startup rendered its UI; log had no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/cxbottle --bottle Shandalar-Win8-Test --create --template win8 --description "Shandalar Win8 test bottle"` | repo root | Created a fresh 32-bit Win8 bottle. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-win8test-startup-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "Y:\Shandalar\Shandalar\Shandalar.exe"` | repo root | Stayed alive until cleanup after the timed smoke; log showed `wined3d`, successful `NtGdiCreateDIBSection` calls, and no startup page fault. |
| `cp -cR . "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar"` | repo root | Created a bottle-local install copy for `C:\Shandalar`; `.git/fsmonitor--daemon.ipc` was a socket and was not copied. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-tall-startup-cx.log --debugmsg +seh,+bitmap,+process "C:\Shandalar\Shandalar.exe"` | repo root | Stayed alive until cleanup after the timed smoke; log used the bottle-local `C:\Shandalar` path and showed no startup page fault. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-repo-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "Y:\Shandalar\Shandalar\Shandalar.exe"` plus `wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"` | repo root | Passed the reported crash point; log showed post-color loads of `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`, with no original page fault/assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"` plus the same `wscript` helper | repo root | Passed the reported crash point from the normal C-drive install path; log showed the same post-color resource loads with no original page fault/assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"` plus the same `wscript` helper | repo root | Passed the reported crash point from the older practical `MTG` shortcut path; log showed the same post-color resource loads with no original page fault/assertion. |
| Visual smoke launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | repo root | Reached the `Magic: Shandalar` main menu; macOS denied synthetic keypresses, so Start New Game was not tested. |
| Direct logged launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` | repo root | Loader failure before gameplay because `Program\zlib.dll` is missing, cascading through `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll`. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Shandalar.exe --help` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Shandalar.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Magic.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |

These results prove the commands were attempted in the existing local `MTG`
bottle. They do not prove that the game fails in all CrossOver bottles.

## Test Matrix

| Platform/bottle | Executable | Expected check | Status |
| --- | --- | --- | --- |
| Windows native | Root `Shandalar.exe` | Main Shandalar UI opens, can start or load game. | Needs testing. |
| Windows native | Root `Magic.exe` | Duel shell opens or reports a visible dependency/config issue. | Needs testing. |
| Windows native | `Program/Magic.exe` | Manalink/duel shell opens. | Needs testing. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | Automated SendKeys smoke passed the reported crash point from the practical shortcut path before the name-entry and movement patches; full visible gameplay still needs manual testing. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\FaceMaker.exe` | Character helper opens and creates its DIB surfaces. | Direct helper launch rendered successfully; Shandalar-spawned character creation still needs visible testing. |
| CrossOver `Shandalar-Win8-Test` bottle | patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | Automated SendKeys smoke passed the reported crash point before the name-entry and movement patches; full visible gameplay still needs manual testing. |
| CrossOver `Shandalar-Win8-Test` bottle | patched `C:\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | Automated SendKeys smoke passed the reported crash point from the normal C-drive install path before the name-entry and movement patches; full visible gameplay still needs manual testing. |
| CrossOver `MTG` bottle | `C:\Shandalar\Magic.exe` | Root duel executable opens. | Needs visible testing. |
| CrossOver `MTG` bottle | patched root `C:\Shandalar\ManalinkEh.dll` with `C:\Shandalar\Magic.exe` | Femeref/Samite/Kithkin healer damage prevention only appears during the engine damage-prevention window; duel no longer freezes after Femeref Healer responds to blockers. | Repo and copied `MTG` bottle DLLs are patched with backups preserved; visible retest still needed. |
| CrossOver `MTG` bottle | patched root `C:\Shandalar\Magic.exe` | During declare attackers, clicking an already-declared ordinary attacker before Done removes it from the declared attackers selection. | Repo root and copied `MTG` bottle `Magic.exe` are patched; visible retest still needed. |
| CrossOver `MTG` bottle | `C:\Shandalar\Program\Magic.exe` | Program Manalink executable opens. | Needs visible testing. |
| CrossOver XP bottle | root `Shandalar.exe` from copied install or repo root | UI opens, start-color flow works. | Needs testing. |
| CrossOver XP bottle | `Program/Magic.exe` | UI opens from `Program/` working directory. | Needs testing. |
| CrossOver Windows 7 bottle | both | Compare to XP. | Needs testing. |
| CrossOver Windows 8 bottle | both | Compare to the forum thread's Windows 8 success clue. | `Shandalar-Win8-Test` created; patched repo Shandalar passed the crash-point smoke test. |
| CrossOver Windows 10 bottle | both | Try if XP/7 do not work. | Needs testing. |
| CrossOver Windows 11 bottle | both | Comparison/fallback only. | Needs testing. |

## Capturing Logs

| Environment | Practical capture method |
| --- | --- |
| CrossOver GUI | Use Run Command, enable logging if offered, and save the log path with bottle name and Windows version. |
| CrossOver CLI | Run from Terminal and redirect output outside the repo if needed: `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME Magic.exe >~/Desktop/magic.log 2>&1`. |
| Wine CLI | Run with Wine debug channels only when needed: `WINEDEBUG=+loaddll,+seh wine Magic.exe`. |
| Windows | Check Event Viewer only if the app exits silently. Also note any missing DLL dialog exactly. |

## Needs testing

`Shandalar.exe --help` may show a dialog, write to a log, or require a console
wrapper. This pass did not capture help text.
