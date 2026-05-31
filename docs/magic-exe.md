# Magic.exe

`Magic.exe` is a first-class launch target in this checkout. It is not just a
helper for `Shandalar.exe`.

## What It Appears to Be

| Evidence | Interpretation |
| --- | --- |
| `Manalink_Launcher.cmd:100-102` changes to `Program/` and starts `Magic.exe`. | The launcher treats `Magic.exe` as the Manalink play target. |
| `Manalink3/Program/ReadMe.txt:86-89` says `Play_Manalink.cmd` enters `.\Program` and executes `Magic.exe`. | Manalink packaging expected `Magic.exe` under `Program/`. |
| `README - ManalinkEx.txt:5-8` says `Magic.exe` is patched to load `ManalinkEx.dll` and that the editor needs `magic.exe` plus `manalink.csv`. | This `Magic.exe` is part of the patched Manalink/card-engine flow. |
| Import table includes `manalinkeh.dll` and `manalinkex.dll`. | These DLLs must be available next to or loadable by `Magic.exe`. |
| Logged CrossOver `MTG` startup of root `C:\Shandalar\Shandalar.exe` opens `C:\Shandalar\Magic.exe`. | Root `Magic.exe` is also a real runtime target and must be tested separately from `Program/Magic.exe`. |

## Binary Facts

| File | Fact |
| --- | --- |
| `Program/Magic.exe` | PE32 GUI Intel 80386, 32-bit Windows executable. |
| Root `Magic.exe` | PE32 GUI Intel 80386, same PE timestamp/import set as `Program/Magic.exe`, different SHA-256; opened by root Shandalar startup in bottle `MTG`. |
| `Program/Magic.exe` imports | `user32.dll`, `advapi32.dll`, `comctl32.dll`, `deckdll.dll`, `drawcardlib.dll`, `gdi32.dll`, `kernel32.dll`, `msvcrt.dll`, `msvfw32.dll`, `version.dll`, `winmm.dll`, `comdlg32.dll`, `manalinkeh.dll`, `manalinkex.dll`. |

## Current Runtime Patches

| Patch | Root `Magic.exe` | `Program/Magic.exe` | Notes |
| --- | --- | --- | --- |
| Declared-attacker undo | Hook `0x43c303`, cave `0x459bc8`, SHA-256 `5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1`. | Hook `0x43c303`, cave `0x459bc8`, SHA-256 `0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa`. | Lets the human player click an already-declared attacker before Done to clear `STATE_ATTACKING`. See [bugs/declared-attacker-undo.md](bugs/declared-attacker-undo.md). |

## Required Nearby Files

| File/folder | Why |
| --- | --- |
| `Program/ManalinkEh.dll` | Direct import. |
| `Program/ManalinkEx.dll` | Direct import. |
| `Program/Deckdll.dll` | Direct import. |
| `Program/Drawcardlib.dll` | Direct import. |
| `Program/CardArtLib.dll` | Used by related DLLs and runtime art flow. |
| `Program/Cards.dat`, `Program/Text.res`, `Program/Rarity.dat`, `Program/DBInfo.dat` | Card database and string/resource data. |
| `Program/CardArt`, `Program/DuelArt`, `Program/DuelSounds`, `Program/Sound`, `Program/PlayFace`, `Program/decks` | Runtime assets and decks. |

## Verified on this machine

Command:

```sh
cd /Users/mdmoll/Shandalar/Shandalar/Program
/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Magic.exe
```

Result: exit code 53, no stdout/stderr captured. This is a local CrossOver
`MTG` bottle result, not a definitive `Magic.exe` failure.

## Why Magic.exe Might Fail When Shandalar.exe Works

| Hypothesis | Evidence | Test |
| --- | --- | --- |
| Missing `ManalinkEh.dll` or `ManalinkEx.dll` | Direct imports of `Program/Magic.exe`. | Confirm both DLLs are in the working directory and watch for missing-DLL dialogs/logs. |
| Wrong working directory | Launcher docs and scripts enter `Program/` before starting `Magic.exe`. | Launch from `Program/`, not by double-clicking a copied exe elsewhere. |
| Old multimedia/video dependency | Imports `MSVFW32.dll` and `WINMM.dll`. | Capture exact error; try virtual desktop or codecs only after UI/log evidence. |
| Registry expectations | Imports `advapi32.dll` registry APIs. | Compare clean bottle vs bottle where the game has been launched once. |
| Display/palette mode issue | Uses GDI/user32 and old UI assets; no direct D3D/DDRAW imports found. | Try Wine virtual desktop at 800x600 or 1024x768. |
| Root/Program binary difference | Root and Program `Magic.exe` have different SHA-256. | Test both paths separately and record hashes. |
| CrossOver desktop and Windows version behavior | Bottle `MTG` currently sets app-default `Version=win7` and desktop `Shandalar1440=1440x1080` for `Magic.exe` as the larger 4:3 retest setting. | Retest root and `Program` `Magic.exe` with that setting before changing DLLs or runtimes. |

## Recommended Manual Test

1. In CrossOver, use the existing `MTG` bottle or create a fresh 32-bit XP or
   Windows 7 bottle.
2. For bottle `MTG`, use Run Command with:

```text
Command: C:\Shandalar\Magic.exe
Working directory: C:\Shandalar
```

3. Repeat for the Manalink `Program` copy:

```text
Command: C:\Shandalar\Program\Magic.exe
Working directory: C:\Shandalar\Program
```

4. Record whether the UI appears, whether a missing-DLL dialog appears, and
where CrossOver writes the log.
5. Repeat for root `C:\Shandalar\Shandalar.exe` in the same bottle.
