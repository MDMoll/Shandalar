# CrossOver on macOS

This page is a practical recipe for testing the local checkout through
CrossOver. It is not proof that the game only works this way.

Verified local CrossOver helper:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --version
```

Result on this machine: CrossOver 26.1.0.39808.

## Bottle Recipe

| Step | Recommendation |
| --- | --- |
| 1 | Create separate 32-bit bottles for Windows XP, Windows 7, Windows 8, and Windows 10 tests. Use Windows 11 only for comparison. |
| 2 | Start with no extra redistributables. The inspected launch targets import `msvcrt.dll`, not `vcruntime*.dll`, `msvcp*.dll`, or `mfc*.dll`. |
| 3 | If a missing-runtime dialog appears, install x86 runtimes into the bottle, not the repo. Try Visual C++ 2010 x86 only after recording the exact missing DLL or old-doc rationale. |
| 4 | For the current copied `MTG` install, use `C:\Shandalar\Shandalar.exe` with working directory `C:\Shandalar`. Direct `C:\Shandalar\Program\Shandalar.exe` previously exposed missing adjacent layout files: first `Program\zlib.dll`, then Program CardArt assets, `Program\Manalink.ini`, Program `DuelArt` configs through `Planeswalker.dat`, six Program `TT*.ttf` font files, and older Program card-data files. This checkout and the local copied install now have those files; the latest bounded Program-path log opened the Program card-data trio, `shandalar.dll`, and `Shandalar.ini` without the earlier fatal strings, but the exact Program path still needs visible retesting. |
| 5 | Save separate launchers for root `Shandalar.exe`, root `Magic.exe`, and `Program\Magic.exe` only after each one works. |
| 6 | Leave high-resolution or Retina mode off for first launch unless testing proves it helps; use Wine virtual desktop for the Shandalar start-color path. |
| 7 | For the start-color assertion, make sure `FaceMaker.exe`, `FaceData.txt`, `FaceButtons.txt`, and face art are present next to the active Shandalar launch path, but do not treat FaceMaker alone as proven sufficient. |
| 8 | Set `Window = 2` in both `Shandalar.ini` and `Program/Shandalar.ini` before the start-color retest. |
| 9 | Give the bottle a non-tiny paging file, such as `C:\pagefile.sys 512 1024`, because the forum thread identifies paging-file absence as a trigger. |
| 10 | Use the patched `Shandalar.exe` for the start-color assertion. The local `MTG` and `Shandalar-Win8-Test` copied installs were patched in this pass, with original backups saved beside each executable. |
| 11 | For duel-freeze retests, use the patched `ManalinkEh.dll` too. The local `MTG` copied install has already been updated with backups for the Femeref/Samite/Kithkin healer guard, generic activated damage-prevention guard, AI decision-time clamp, AI raw-mana snapshot patch, Piranha Marsh trigger-target patch, Bojuka Bog trigger-target patch, generic AI player-target selector patch, and AI ETB player-target trigger-mode handling patch. Repeat the backup/copy pattern for any other copied CrossOver install only with explicit approval before retesting there. |
| 12 | For duel-start retests, keep `ShowCoinFlips=0`. A bounded `--e 0442 --p 0442` log in `MTG` opened root `Magic.exe` without targeted coin-flip/fatal strings, but visible first-turn input still needs testing. |

## Run Command Values

The current local `MTG` bottle has a copied install at `C:\Shandalar`. Its
CrossOver shortcut targets the root Shandalar executable, and logged startup
shows that root `Shandalar.exe` opens root `Magic.exe`.

| Target | Command | Working directory | Status |
| --- | --- | --- |
| Shandalar in copied `MTG` bottle | `C:\Shandalar\Shandalar.exe` | `C:\Shandalar` | Bottle-local copy is now patched; config-only fixes did not solve user retesting before this patch. |
| Root Magic in copied `MTG` bottle | `C:\Shandalar\Magic.exe` | `C:\Shandalar` | Opened by root `Shandalar.exe`; a bounded `--e 0442 --p 0442` direct-duel log also opened it without targeted coin-flip/fatal strings. Root `ManalinkEh.dll` is patched for damage-prevention, AI clamp, AI raw-mana snapshot, Piranha Marsh/Bojuka Bog trigger-target retests, generic AI player-only target selection, and AI ETB player-target trigger-mode handling. |
| Program Magic in copied `MTG` bottle | `C:\Shandalar\Program\Magic.exe` | `C:\Shandalar\Program` | Manalink launcher path; `Program\ManalinkEh.dll` is patched for equivalent damage-prevention, AI clamp, AI raw-mana snapshot, Piranha Marsh/Bojuka Bog trigger-target testing, generic AI player-only target selection, and AI ETB player-target trigger-mode handling. |
| Program Shandalar in copied `MTG` bottle | `C:\Shandalar\Program\Shandalar.exe` | `C:\Shandalar\Program` | Earlier failures exposed missing `Program\zlib.dll`, missing Program CardArt drawcardlib assets, `Program\Manalink.ini`, Program `DuelArt` configs through `Planeswalker.dat`, Program `TT*.ttf` font files, and older Program card-data files. Repo and copied-install layouts now include them, and the latest bounded log opened the Program card-data trio, `shandalar.dll`, and `Shandalar.ini` without the earlier fatal strings. The exact path still needs a visible retest. |
| Shandalar in fresh Win8 test bottle | `C:\Shandalar\Shandalar.exe` | `C:\Shandalar` | Bottle-local copy is now patched and passed the crash-point smoke test. |
| Patched fresh Win8 repo-mapped launch | `Y:\Shandalar\Shandalar\Shandalar.exe` | `Y:\Shandalar\Shandalar` | Start-color crash point verified with Wine `wscript` SendKeys after the repo binary patch. |

CLI form:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\FaceMaker.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" "Y:\Shandalar\Shandalar\Shandalar.exe"
```

Logging form used for the current investigation:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-cdrive-appdefault-desktop-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\Shandalar.exe"
```

Direct-duel smoke form used after the coin-flip default patch:

```sh
/usr/bin/perl -e 'alarm 25; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-root-ep0442-coinflip-default-cx.log --debugmsg +seh,+file,+process "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442
```

The command printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"`,
opened root `Magic.exe`, and had no targeted coin-flip/fatal strings in the log.
The child process stayed alive until the alarm and was killed manually.

For a fresh bottle that launches directly from the macOS checkout instead of a
copied `C:\Shandalar` tree, start with the repo root for Shandalar:

```sh
cd /Users/mdmoll/Shandalar/Shandalar
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME Shandalar.exe
```

## Changing the Reported Windows Version

Use Wine Configuration inside the bottle:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME winecfg
```

In the Applications tab, choose the Windows version for the bottle or for the
specific executable. Record exactly what you changed.

The local `MTG` bottle is currently being tested with app-specific Windows 7
compatibility for the three executables in the Shandalar/duel path:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe" /v Version /t REG_SZ /d win7 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe" /v Version /t REG_SZ /d win7 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe" /v Version /t REG_SZ /d win7 /f
```

Earlier Windows 8 app-default testing did not prove a durable fix in `MTG`.
Use `Shandalar-Win8-Test` only as the cleaner Win8 comparison bottle.

## Fresh Win8 Test Bottle

The fresh bottle was created with:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/cxbottle --bottle Shandalar-Win8-Test --create --template win8 --description "Shandalar Win8 test bottle"
```

It was initially configured with:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" /v PagingFiles /t REG_MULTI_SZ /d "C:\pagefile.sys 512 1024" /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\Explorer\Desktops" /v Shandalar /t REG_SZ /d 1024x768 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\Magic.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
```

The checkout was then copied into the bottle so the preferred test no longer
depends on CrossOver's macOS `Y:` drive mapping:

```sh
cp -cR . "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar"
```

Verified caveat: `.git/fsmonitor--daemon.ipc` was a socket and was not copied.
That does not affect game runtime files.

After startup logs showed successful 1024-wide and 800-high-ish DIB creation,
the app-default desktop was changed to `ShandalarTall=1024x800`:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\Explorer\Desktops" /v ShandalarTall /t REG_SZ /d 1024x800 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe\Explorer" /v Desktop /t REG_SZ /d ShandalarTall /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\Magic.exe\Explorer" /v Desktop /t REG_SZ /d ShandalarTall /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe\Explorer" /v Desktop /t REG_SZ /d ShandalarTall /f
```

Verified registry evidence:

| Check | Result |
| --- | --- |
| `rg -n "Template|WineArch" ".../Bottles/Shandalar-Win8-Test/cxbottle.conf"` | `Template=win8`, `WineArch=win32`. |
| `grep -aEn 'CurrentBuild|CurrentVersion|ProductName' ".../Bottles/Shandalar-Win8-Test/system.reg"` | `CurrentBuild=9200`, `CurrentVersion=6.2`, `ProductName=Microsoft Windows 8`. |
| `grep -aEn 'PagingFiles' ".../Bottles/Shandalar-Win8-Test/system.reg"` | `C:\pagefile.sys 512 1024`. |
| `ls -ld ".../Bottles/Shandalar-Win8-Test/drive_c/Shandalar"` | Bottle-local `C:\Shandalar` install copy exists. |
| `rg -n "ShandalarTall|Desktop" ".../Bottles/Shandalar-Win8-Test/user.reg"` | `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe` use desktop `ShandalarTall`; `ShandalarTall=1024x800`. |

## Patched Start-Color Test

The start-color assertion fix is currently a binary patch in this checkout,
not a CrossOver setting. The first patch changes root `Shandalar.exe` and
`Program/Shandalar.exe` at file offset `0x1785b0` so `CreateDIBSection` receives
`hSection = NULL`. After the user still reproduced the issue, the active root
and `Program` FaceMaker helpers were patched the same way at their own
`CreateDIBSection` wrapper, file offset `0x5f40`. A later Shandalar follow-up
patch at file offset `0xa1a42` seeds the name-entry buffer with `mPlayer`
instead of copying the initial name out of the fragile name-picker image
surface.
The latest name follow-up bypasses the fragile manual name editor at file
offset `0xa1acd` and adds an empty-name fallback through code cave
`0x465170`/file offset `0x64570`.
The latest follow-up adds an adventure-map movement patch: hooks at `0x44398c`,
`0x444a2b`, and `0x444aa7` use code cave `0x46502d` and flag `0x583a2c` so a
repeat press of the active movement key can route through the existing stop
path at `0x444a96`.

The current active root and `Program` Shandalar files hash to:

```text
ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b
```

The earlier hSection-only Shandalar patch hashed to
`73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`; keep
that value only as historical evidence.

The patched active FaceMaker helpers hash to:

```text
41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246
```

The unpatched no-resolution/Korath FaceMaker reference helpers still hash to:

```text
43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b
```

Verified smoke commands:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-repo-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "Y:\Shandalar\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-direct-after-patch-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\FaceMaker.exe"
```

Verified result: both `/tmp/shandalar-repo-patched-sendkeys-cx.log` and
`/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` show `advfac64.pic`,
`magic3.map`, `magic4.map`, and `begin.spr` loaded after the post-color DIB
creation path, with no `Unhandled exception`, page fault, or
`WM_CREATE CreateDIBSection` assertion. The older practical `MTG` path also
passed the same crash-point smoke in `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log`.

The local copied installs were patched with backups:

```text
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.before-hsection-null-patch.exe
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.before-hsection-null-patch.exe
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Shandalar.before-hsection-null-patch.exe
/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/Shandalar.before-hsection-null-patch.exe
```

Needs testing: full visible gameplay after character creation, including
default `Player` name acceptance, same-arrow map stop, save/load, and a duel.
The direct patched FaceMaker run proves the helper can render on its own; it
does not prove the Shandalar-spawned character-creation path.

## Current MTG 1440x1080 Test Setting

Needs testing: the user reported that after two or three turns, duel prompts
such as `Done`, `Trigger`, and `Decline` stop accepting input. Fullscreen also
felt too small or fragile on a 27-inch 5K display. The current `MTG` bottle
setting keeps Windows 7 compatibility and uses a conservative 4:3 Wine virtual
desktop named `Shandalar1440` at `1440x1080`.

Verified commands:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\Explorer\Desktops" /v Shandalar1440 /t REG_SZ /d 1440x1080 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe" /v Version /t REG_SZ /d win7 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe" /v Version /t REG_SZ /d win7 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe" /v Version /t REG_SZ /d win7 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar1440 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar1440 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar1440 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\MicroProse\Magic: The Gathering\DuelOptions" /v ShowCoinFlips /t REG_SZ /d 0 /f
```

Verified registry evidence:

```sh
sed -n '260,280p;790,825p;965,970p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"
```

Optional local verifier for the current `MTG` bottle state:

```sh
tools/verify-crossover-mtg-state.sh
```

This checks the copied `C:\Shandalar` runtime hashes, representative Magic and
Manalink patch bytes, FaceMaker support files, `Window = 2`,
`AiDecisionTime=270`, app-default `win7`, `ShowCoinFlips=0`,
`Shandalar1440=1440x1080`, root/Program zlib hashes, Program adjacent
config/font files, Program CardArt drawcardlib assets, and the paging-file
registry setting. It does not launch the game or prove gameplay.

Expected entries:

```text
[Software\\MicroProse\\Magic: The Gathering\\DuelOptions]
"ShowCoinFlips"="0"

[Software\\Wine\\AppDefaults\\FaceMaker.exe]
"Version"="win7"

[Software\\Wine\\AppDefaults\\FaceMaker.exe\\Explorer]
"Desktop"="Shandalar1440"

[Software\\Wine\\AppDefaults\\Magic.exe]
"Version"="win7"

[Software\\Wine\\AppDefaults\\Magic.exe\\Explorer]
"Desktop"="Shandalar1440"

[Software\\Wine\\AppDefaults\\Shandalar.exe]
"Version"="win7"

[Software\\Wine\\AppDefaults\\Shandalar.exe\\Explorer]
"Desktop"="Shandalar1440"

[Software\\Wine\\Explorer\\Desktops]
"Shandalar1440"="1440x1080"
```

## If the Window Disappears or Opens Offscreen

| Symptom | Try |
| --- | --- |
| Immediate silent exit | Re-run from CrossOver Run Command with logging enabled. Record missing DLL dialogs or exit code. For bottle `MTG`, use root `C:\Shandalar\Shandalar.exe` first. |
| Duel prompts stop accepting `Done`, `Trigger`, or `Decline` | Use the current `MTG` Win7/`Shandalar1440`/`ShowCoinFlips=0` setting above, fully quit any old CrossOver Shandalar/Magic windows, then relaunch root `C:\Shandalar\Shandalar.exe` from working directory `C:\Shandalar`. The local copied DLLs and Magic executables are already patched; if it still freezes, capture the exact card/phase and a live process sample while frozen. |
| Fullscreen/palette weirdness | Try a virtual desktop in `winecfg`, start with 800x600 or 1024x768. |
| Assertion after choosing a start color | Use the patched `Shandalar.exe` and patched active `FaceMaker.exe`; verify FaceMaker support files, set `Window = 2`, enlarge the bottle paging file, and use a Wine virtual desktop. For the active `MTG` path, keep app-default `Version=win7` and `Shandalar1440=1440x1080`; use `Shandalar-Win8-Test` only as a comparison bottle. |
| Window opens offscreen | Enable virtual desktop, then relaunch from the same folder that contains the executable and DLLs. |
| Desktop resolution changes | Disable high-resolution/Retina mode and use virtual desktop. |
| Sound/video failure after UI opens | The binaries import `WINMM.dll` and `MSVFW32.dll`; capture the exact dialog/log before installing codecs or runtimes. |

## Test Results Template

| Date | CrossOver version | Bottle | Windows version | Executable | Result | Error text | Dependencies changed | Notes/logs |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Not inspected | `Shandalar.exe --help` | Exit 53, no stdout/stderr | None captured | None | Timed CLI run from `Program/`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Not inspected | `Shandalar.exe` | Exit 53, no stdout/stderr | None captured | None | Timed CLI run from `Program/`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Not inspected | `Magic.exe` | Exit 53, no stdout/stderr | None captured | None | Timed CLI run from `Program/`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Windows 7 in registry | Registry only | Updated `PagingFiles` from `C:\pagefile.sys 27 77` to `C:\pagefile.sys 512 1024` | None | Paging-file registry changed with CrossOver `wine reg add` | Visible game retest still needed. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Windows 7 in registry | Copied install config | Set both `C:\Shandalar\Shandalar.ini` and `C:\Shandalar\Program\Shandalar.ini` to `Window = 2` | None | Hashes now match the repo ini files | Visible game retest still needed. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | Windows 7 in registry | `C:\Shandalar\Program\Shandalar.exe` | Loader failure before gameplay | Missing `Program\zlib.dll` cascades into `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll` load failures | None | Historical pre-`Program/zlib.dll` layout fix; local repo and copied install now have matching zlib DLLs, and a later bounded loader smoke loaded the dependent DLL chain. |
| 2026-06-02 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Timed loader-path smoke | No old missing-zlib/import-failure/page-fault pattern in targeted scan | `/tmp/shandalar-mtg-program-zlib-retest-cx.log` | Program `zlib.dll`, `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll` loaded natively; run timed out as a GUI smoke and leftover processes were killed. Not visible gameplay proof. |
| 2026-06-03 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Visible dialog from `drawcardlib.dll` | `Could not load ManaSymbols= "C:\Shandalar\Program\CARDART\ManaSymbols.pic"` | Copied `ManaSymbols.pic`, `Expansion_Symbols.pic`, `Watermarks.pic`, and `CardCounters.png` from the preserved `Mods/Art/_undo/.../CardArt` snapshot into repo `Program/CardArt` and the local copied install. | Static hashes now match [runtime-manifest.md](runtime-manifest.md); later bounded exact-path logs reached missing `Modern/Triggering.png`, `Planeswalker/LoyaltyBase.png`, and `Modern/CardOv_Nyx.png`, now also copied. Exact-path visible retest still needed. |
| 2026-06-03 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after first Program CardArt copy | Missing `C:\Shandalar\Program\CARDART\modern\Triggering.png` in `/tmp/shandalar-mtg-program-cardart-retest-cx.log` | Copied `Modern/Triggering.png` from the same preserved Program-style snapshot into repo `Program/CardArt/Modern` and the local copied install. | `Program/CardArt/Modern` now matches the preserved snapshot by `diff -qr`. |
| 2026-06-03 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `Triggering.png` copy | Missing `C:\Shandalar\Program\CARDART\planeswalker\LoyaltyBase.png` in `/tmp/shandalar-mtg-program-cardart-triggering-retest-cx.log` | Copied `LoyaltyBase.png`, `LoyaltyMinus.png`, `LoyaltyPlus.png`, and `LoyaltyZero.png` from the same preserved Program-style snapshot into repo `Program/CardArt/Planeswalker` and the local copied install. | No further visible Program-path retest has been run. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after loyalty-image copy | Missing `C:\Shandalar\Program\CARDART\modern\CardOv_Nyx.png` in `/tmp/shandalar-mtg-program-cardart-loyalty-retest-cx.log` | Copied `CardOv_Nyx.png` from the preserved Program-style `Planeswalker` frame folder into repo `Program/CardArt/Modern` and the local copied install. | `Program/DuelArt/Duel.dat` maps the Modern `CardOv_*Nyx` entries to this generic overlay. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `CardOv_Nyx.png` copy | Missing `C:\Shandalar\Program\Manalink.ini` in `/tmp/shandalar-mtg-program-cardart-nyx-retest-cx.log` | Copied active root `Manalink.ini` into repo `Program/Manalink.ini` and the local copied install. | Follow-up log opened the Program path successfully. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `Program/Manalink.ini` copy | Missing `C:\Shandalar\Program\DuelArt\Modern.dat` in `/tmp/shandalar-mtg-program-manalinkini-retest-cx.log` | Copied `Modern.dat` from the preserved Program-style `Mods/Art/_undo/.../DuelArt` snapshot into repo `Program/DuelArt` and the local copied install. | Follow-up log opened `Modern.dat` and exposed the next Program adjacent gaps. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `Modern.dat` copy | Missing `C:\Shandalar\Program\DuelArt\Planeswalker.dat` and Program `TT*.ttf` font files in `/tmp/shandalar-mtg-program-moderndat-retest-cx.log` | Copied `Planeswalker.dat` from the preserved Program-style `Mods/Art/_undo/.../DuelArt` snapshot and copied the six root font files into Program under the exact hardcoded `TT*.ttf` names; mirrored all into the local copied install. | No new drawcardlib missing-asset dialog or page fault found in the targeted scan. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `Planeswalker.dat` and font copies | No new Program data/art/font missing path in targeted scan; run exited with code 1 | None after this run | `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log` opened `Program\DuelArt\Modern.dat`, `Program\DuelArt\Planeswalker.dat`, the six Program `TT*.ttf` font paths, and then `Program\Magic.exe`. This is loader/config evidence only, not visible gameplay proof. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Visible Program-path fatal after adjacent config/font/art copies | `Bad raw_cards_data[-1].card() building Hornet: expected 15835, got -1` | Copied root `Cards.dat` and `Rarity.dat` into repo `Program/` and the local copied install. | The pre-fix Program `Cards.dat`/`Rarity.dat` files had 15718 records, which was too short for expected id 15835. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after card-data copy | No `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, or unhandled-exception strings found in targeted scan | None after this run | `/tmp/shandalar-mtg-program-carddata-retest-cx.log` opened `C:\Shandalar\Program\Cards.dat` with `ret 0`. The process stayed alive until the 20-second alarm and was killed; this is specific-fatal evidence only, not visible gameplay proof. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Visible Program-path fatal after `Cards.dat`/`Rarity.dat` copies | `Fatal: Couldn't find Cards.dat, DBInfo.dat or Rarity.dat` | Copied root `DBInfo.dat` into repo `Program/` and the local copied install. | `DeckDLL` returns this same fatal when `DBInfo.dat` opens but its record count does not match `Cards.dat`; the pre-fix Program `DBInfo.dat` header had 15718 records and now matches root at 16818. Exact-path visible retest still needed. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after `DBInfo.dat` copy | No generic card-data fatal, `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, or unhandled-exception strings found in targeted scan | None after this run | `/tmp/shandalar-mtg-program-dbinfo-retest-cx.log` opened `C:\Shandalar\Program\DBInfo.dat` with `ret 0`. The process stayed alive until the 20-second alarm and was killed; cleanup tail includes `wineserver crashed`, so this is specific-fatal evidence only, not visible gameplay proof. |
| 2026-06-04 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Program\Shandalar.exe` | Bounded logged retest after full Program card-data trio | No generic card-data fatal, `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, assertion, or unhandled-exception strings found in targeted scan | None after this run | `/tmp/shandalar-mtg-program-post-dbinfo-visible-retest-cx.log` opened `C:\Shandalar\Program\Cards.dat`, `DBInfo.dat`, `Rarity.dat`, `shandalar.dll`, and `Shandalar.ini`. The child process stayed alive past the 20-second alarm and was killed manually, so this is loader/fatal-regression evidence only, not visible gameplay proof. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | Registry only | Set app-default virtual desktop for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`; set app-default Windows 8 for all three | None | `HKCU\Software\Wine\AppDefaults\*.exe` plus `HKCU\Software\Wine\Explorer\Desktops` | Persistent setting verified in `user.reg`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | `C:\Shandalar\Shandalar.exe` | Stayed alive until timed alarm | None in startup log | Uses app-default virtual desktop `Shandalar` at `1024x768` | Log showed `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. Visible start-color retest still needed. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | `C:\Shandalar\FaceMaker.exe` | Stayed alive until timed alarm | None in startup log | Uses app-default virtual desktop `Shandalar` at `1024x768` | Direct FaceMaker log showed 1024x768 8bpp DIB sections; this does not prove the Shandalar-spawned character-creation path. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | `C:\Shandalar\Shandalar.exe` | Visible main menu reached | None visible | Same settings as above | macOS denied synthetic keystrokes from this session, so Start New Game was not tested. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | User retest | Still reproduced start-color issue | `WM_CREATE CreateDIBSection` class of failure | Same settings as above | App-default Win8 in a Windows 7 bottle is not sufficient evidence of a fix. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Bottle creation | Created successfully | None | Fresh bottle, pagefile, virtual desktop | `cxbottle.conf` reports `Template=win8`, `WineArch=win32`; registry reports Microsoft Windows 8. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | `Y:\Shandalar\Shandalar\Shandalar.exe` | Stayed alive until cleanup after timed smoke | None in startup log | Uses virtual desktop `Shandalar=1024x768` | `/tmp/shandalar-win8test-startup-cx.log` showed `wined3d` and successful DIB creation. Start-color still needs manual testing. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Bottle-local copy | Copied checkout to `C:\Shandalar` | `.git/fsmonitor--daemon.ipc` socket not copied | None | This makes the next test closer to a normal Windows install path. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Registry only | Set app-default desktop `ShandalarTall=1024x800` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` | None | Virtual desktop changed from earlier `Shandalar=1024x768` | This is based on logged DIB sizes; it is not yet a proven fix. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | `C:\Shandalar\Shandalar.exe` | Stayed alive until cleanup after timed smoke | None in startup log | Uses bottle-local `C:\Shandalar` and `ShandalarTall=1024x800` | `/tmp/shandalar-win8test-cdrive-tall-startup-cx.log` showed no startup page fault. Start-color still needs manual testing. |
| 2026-05-30 | 26.1.0.39808 | `MTG` and `Shandalar-Win8-Test` | Bottle copies | Patched copied `Shandalar.exe` files | Completed | None | Original copies backed up as `Shandalar.before-hsection-null-patch.exe` | Four bottle-local `Shandalar.exe` copies now hash to the combined-patch value `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; the older hSection-only value was `73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` and `Shandalar-Win8-Test` | Bottle copies | Copied movement-stop patched `Shandalar.exe` files | Completed | None | Same active binaries as repo root and `Program/` | Four bottle-local `Shandalar.exe` copies now hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; same-arrow stop still needs visible gameplay testing. |
| 2026-05-30 | 26.1.0.39808 | `MTG` and `Shandalar-Win8-Test` | Bottle copies | Copied name-editor bypass/fallback patched `Shandalar.exe` files | Completed | None | Same active binaries as repo root and `Program/` | Four bottle-local `Shandalar.exe` copies now hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; this row was superseded by the later 2026-05-31 visible S2 run that reached the map in `MTG`. |
| 2026-05-31 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\Shandalar.exe` | Reached adventure map for default/first start-color path | None visible | Wine `wscript` SendKeys plus `Shandalar1440=1440x1080` | S2 is recorded in [manual-gameplay-verification.md](manual-gameplay-verification.md); cropped screenshot is [generated/manual-gameplay/s2-map-2026-05-31.png](generated/manual-gameplay/s2-map-2026-05-31.png). Raw local log `/tmp/shandalar-visible-s2-attempt-cx.log` observed `FaceMaker-nores.exe /S`. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Patched `C:\Shandalar\Shandalar-nosection-test.exe` | Passed reported crash point | None in smoke log | Binary patch forces `hSection = NULL` for `CreateDIBSection` | `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` showed post-color loads of `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` | Passed reported crash point | None in smoke log | Same binary patch, tested from edited repo path | `/tmp/shandalar-repo-patched-sendkeys-cx.log` showed the same post-color resource loads from `Y:\Shandalar\Shandalar`. |
| 2026-05-30 | 26.1.0.39808 | `Shandalar-Win8-Test` | Windows 8 / 32-bit | Patched `C:\Shandalar\Shandalar.exe` | Passed reported crash point | None in smoke log | Same binary patch, tested from normal copied install path | `/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` showed the same post-color resource loads from `C:\Shandalar`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | Patched `C:\Shandalar\Shandalar.exe` | Passed reported crash point | None in smoke log | Same binary patch, tested from the older practical shortcut path | `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log` showed the same post-color resource loads from `C:\Shandalar`. |
| 2026-05-30 | 26.1.0.39808 | `MTG` | App-default `win8` | Patched `C:\Shandalar\FaceMaker.exe` | Direct helper UI rendered | None in smoke log | Active FaceMaker patch forces `hSection = NULL` for its own `CreateDIBSection` wrapper | `/tmp/facemaker-direct-after-patch-cx.log` had no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |

| 2026-05-31 | 26.1.0.39808 | `MTG` | App-default `win7` | Registry only | Set `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` to desktop `root` instead of virtual desktop `Shandalar` | None | `HKCU\Software\Wine\AppDefaults\*.exe` | Superseded by the later `Shandalar1440` setting after fullscreen/root-desktop testing was undesirable; visible gameplay retest was not completed for this row. |
| 2026-05-31 | 26.1.0.39808 | `MTG` | App-default `win7` | Registry only | Set `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` to desktop `Shandalar1440=1440x1080` | None | `HKCU\Software\Wine\Explorer\Desktops` and `HKCU\Software\Wine\AppDefaults\*.exe` | This is the current 4:3 size-up test after fullscreen was undesirable; visible gameplay retest still needed. |
| 2026-05-31 | 26.1.0.39808 | Repo | N/A | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched for Femeref/Samite/Kithkin healer damage-prevention activation freeze | None from static verification | Binary DLL patch plus matching source guards | `xxd` and `objdump` verify the new `LCBP_DAMAGE_PREVENTION` gate. Later repo hashes also include the generic damage-prevention guard, AI clamp patch, AI raw-mana snapshot patch, Piranha Marsh/Bojuka Bog trigger-target patches, generic AI player-target selector patch, and AI ETB trigger-mode patch. |
| 2026-05-31 | 26.1.0.39808 | `MTG` | App-default `win7` | `C:\Shandalar\ManalinkEh.dll` and `C:\Shandalar\Program\ManalinkEh.dll` | Copied patched healer DLLs into the active copied install | None from copy/hash verification | Backups preserved as `*.before-femeref-healer-patch.dll` | Later damage-prevention, AI clamp, AI raw-mana snapshot, Piranha Marsh/Bojuka Bog trigger-target, generic AI player-target selector, and AI ETB trigger-mode patches updated these same DLLs again with additional backups; visible gameplay retest still needed. |
| 2026-06-04 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched `_check_timer_for_ai_speculation` fallback from 5405 to 270 | None from static verification | Guarded patch helper plus byte/hash checks | `xxd` and `objdump` verify `mov ebx, 0x10e`; this was later superseded by the AI clamp and raw-mana snapshot patch hashes. |
| 2026-06-05 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched `_ai_decision_phase` raw-mana snapshot restore | None from static verification | Guarded patch helper plus byte/hash checks; `tools/verify-crossover-mtg-state.sh` | `xxd` and `objdump` verify both players' raw-mana rows are saved before replacing only the opponent row; visible gameplay retest still needed. |
| 2026-06-05 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched Piranha Marsh ETB target selection | None from static verification | Guarded patch helper plus byte/hash checks; `tools/verify-crossover-mtg-state.sh` | `xxd` and `objdump` verify the trigger calls `pick_player_duh()`; visible Piranha Marsh retest still needed. |
| 2026-06-05 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched Bojuka Bog ETB target selection | None from static verification | Guarded patch helper plus byte/hash checks; `tools/verify-crossover-mtg-state.sh` | `xxd` and `objdump` verify the trigger calls `pick_player_duh()`; manual Bojuka retest later showed this selector-side patch was not sufficient. |
| 2026-06-06 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `ManalinkEh.dll` and `Program/ManalinkEh.dll` | Patched AI ETB player-target trigger-mode handling for Piranha Marsh and Bojuka Bog | None from static verification | Guarded patch helper plus byte/hash checks; `tools/verify-crossover-mtg-state.sh` | Manual Bojuka testing showed the earlier end-trigger suppression candidate was not sufficient. `xxd` and `objdump` now verify the wrapper calls `_duh_mode()` and `_comes_into_play_mode(..., RESOLVE_TRIGGER_AI(player))`; fresh visible Bojuka/Piranha retest still needed. |
| 2026-06-04 | 26.1.0.39808 | Repo and `MTG` | App-default `win7` | `Magic.exe`, `Program/Magic.exe`, and `user.reg` | Patched missing-registry `ShowCoinFlips` default from 1 to 0, and set active `MTG` `ShowCoinFlips=0` | None from static verification | Guarded patch helper plus registry update | `xxd` verifies `c7 05 5c 72 78 00 00 00 00 00`; active bottle hashes match the patched repo Magic hashes. Visible duel-start retest still needed. |

## Needs testing

Run a full visible gameplay pass with the patched `Shandalar.exe` and patched
`ManalinkEh.dll` next. The automated SendKeys tests reached the post-color
resource load point, but they did not prove default-name character creation,
same-arrow map stop, save/load, duels, every starting color, or the
Femeref-Healer-in-combat freeze. For copied CrossOver installs, test the exact
root-vs-Program path explicitly; do not assume they behave the same.
