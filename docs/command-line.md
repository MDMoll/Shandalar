# Command-Line Modes

This pass found evidence of command-line support, but did not capture a complete
`Shandalar.exe --help` page.

## Verified on this machine

| Command | Working directory | Result |
| --- | --- | --- |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Shandalar.exe --help` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured; this was before the root-path/virtual-desktop findings. |
| `sed -n '1,120p' 'Shandalar help.bat'` | repo root | The file contains `shandalar --e 0442 --p 0442`. |
| `strings Program/Shandalar.exe | rg -i -C 3 'one ?deck|starting the duel|mulligan'` | repo root | Found `OneDeck ONEDECK ONE DECK`, `Starting the duel.`, `Doing the mulligan.`, and related direct-duel strings. |
| `/usr/bin/perl -e 'alarm 25; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-root-ep0442-coinflip-default-cx.log --debugmsg +seh,+file,+process "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442` | repo root | Printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"` and opened root `C:\Shandalar\Magic.exe`; the child process stayed alive until the alarm and was killed, so this proves command parsing and launch path only, not visible duel play. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-shandsave-magic3-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\Shandalar.exe" /SHANDSAVE MAGIC3.SVE` | repo root | Reached the main menu rather than directly loading a save. The argument was resolved in the log as `Y:\Shandalar\Shandalar\MAGIC3.SVE` because the file existed in the macOS working directory. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-shandsave-cpath-magic3-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe" /SHANDSAVE "C:\Shandalar\MAGIC3.SVE"` | repo root | The explicit C-drive argument was preserved in the process command line. The log did not prove that `MAGIC3.SVE` was opened or directly loaded. |

## Discovered Flags and Evidence

| Flag/command | Evidence | Meaning | Status | Example |
| --- | --- | --- | --- | --- |
| `--help` | User-provided test command and attempted CrossOver run. | Expected to show help. | Needs manual capture. | `Shandalar.exe --help` |
| `--e 0442` | `Shandalar help.bat:1` plus the 2026-06-04 CrossOver direct-duel log. | Direct-duel deck parameter for one side; based on the flag name, likely the enemy/opponent side. With `0442`, the runtime printed `decks/0442.dck` for one side. | Bounded command-path evidence only; visible duel play still needs testing. | `shandalar --e 0442 --p 0442` |
| `--p 0442` | `Shandalar help.bat:1` plus the 2026-06-04 CrossOver direct-duel log. | Direct-duel deck parameter for one side; based on the flag name, likely the player side. With `0442`, the runtime printed `decks/0442.dck` for one side. | Bounded command-path evidence only; visible duel play still needs testing. | `shandalar --e 0442 --p 0442` |
| `OneDeck` / `ONEDECK` / `ONE DECK` | Strings in `Program/Shandalar.exe`. | Direct-duel mode string or command parser token. | Inferred from strings only. | No verified syntax. |
| `/SHANDSAVE` | String in `Shandalar.exe` and CrossOver attempts. | Save-related internal or command-line token. | Syntax not captured; `/SHANDSAVE MAGIC3.SVE` did not direct-load locally, and explicit `C:\Shandalar\MAGIC3.SVE` preserved the argument but did not prove a load. | No verified syntax. |
| `/AUTOSAVE` | String in `Shandalar.exe`. | Save-related token. | Inferred from strings only. | No verified syntax. |

## Current Best Test Commands

Native Windows:

```bat
cd \path\to\Shandalar
Shandalar.exe --help
Shandalar.exe --e 0442 --p 0442
```

CrossOver:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe" --help
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442
```

Wine:

```sh
cd /path/to/repo
wine Shandalar.exe --help
wine Shandalar.exe --e 0442 --p 0442
```

## What to Capture Next

| Check | Capture |
| --- | --- |
| `--help` opens a dialog | Screenshot or exact text. |
| `--help` prints to console | stdout/stderr text. |
| `--e` / `--p` visible behavior | What deck/opponent appears, whether the first turn starts, and whether input remains responsive. |
| Bad arguments | Any usage/error text. |
| CrossOver/Wine behavior | Bottle name, Windows version, command, exit code, log file. |
