# Stale References

This file lists local references that are obsolete, local-machine-specific,
historically useful, or not currently actionable. Network reachability was not
tested in this pass, so URL status is not presented as live/dead proof unless
the evidence is local.

Search basis:

```sh
rg -n -i --glob '*.txt' --glob '*.md' --glob '*.cmd' --glob '*.bat' --glob '*.ini' --glob '*.pl' --glob '*.url' -e 'https?://|ftp://|www\.|[A-Z]:\\|c:\\|d:\\|e:\\|manalink\.org' .
```

## URLs and Historical References

| Path/line | Reference | Classification | Evidence/status | Suggested handling |
| --- | --- | --- | --- | --- |
| `archive/historical-links/Gatheringnet.url:2` | `http://www.gathering.net/` | Obsolete/historical, not reachability-tested | Old root path was `Gatheringnet.url`; InternetShortcut file. | Archived as history. |
| `archive/historical-links/Microprose.url:2` | `http://www.microprose.com/` | Obsolete/historical, not reachability-tested | Old root path was `Microprose.url`; InternetShortcut file. | Archived as history. |
| `archive/historical-docs/Readme13.txt:19-20`, `archive/historical-docs/Readme13.txt:220` | `www.gathering.net`, `www.ten.net`, `www.microprose.com` | Original-era vendor/community references | Old root path was `Readme13.txt`; original-era readme. | Preserve original context in archive. |
| `archive/historical-docs/Readme132.txt:18-28` | `ftp://ftp.microprose.com/...` patch downloads | Obsolete/historical download links, not reachability-tested | Old root path was `Readme132.txt`; FTP vendor patch links from old update readme. | Mark historical; do not tell users to rely on them. |
| `archive/historical-docs/Readme132.txt:51` | `http://members.home.com/adonald1/magic/default.htm` | Likely obsolete historical homepage, not reachability-tested | Old root path was `Readme132.txt`; Home.com personal page reference. | Archived as history. |
| `archive/historical-docs/Readme201.txt:5` | `http://www.slightlymagic.net/forum/viewforum.php?f=25` | Historical/community reference, not reachability-tested | Old root path was `Readme201.txt`; old Manalink update note. | Keep as reference, but do not make it primary docs. |
| `archive/historical-docs/README (2).txt:2-3` | `mybrillgamesite.com`, Slightly Magic forum topic | Historical/community references, not reachability-tested | Old root path was `README (2).txt`; minimal old readme. | Archived as history. |
| `ManaLink.txt:508` and `Program/ManaLink.txt:508` | `www.manalink.org` for latest `Magic.exe` | Stale in-app update prompt, not reachability-tested | In-game text says latest version is at that URL. | Document as historical; do not present as current update source. |
| `Manalink3/Program/ReadMe.txt:127`, `:141`, `:153` | Slightly Magic forum URLs | Historical/community references, not reachability-tested | Manalink3 readme. | Keep, but make README point to local docs first. |
| `Manalink3/Program/ReadMe.txt:179-188`, `:211` | MicroProse FTP patch links and Home.com page | Obsolete/historical bundled-readme references | Nested Manalink3 readme duplicates older update guidance. | Keep as original documentation, not primary instructions. |
| `Manalink3/Program/ReadMe.txt:919`, `:943`, `:995`, `:1001`, `:1011`, `:1071`, `:1076`, `:1082`, `:1086` | Intel/Creative/Ensoniq/Aztech/ATI/Diamond/Chips/Matrox driver sites | Original-era troubleshooting references | Old Windows 95/98 video/audio advice. | Keep only as historical troubleshooting context. |
| `Duelsounds/sounds.txt:3-19`, `Program/DuelSounds/sounds.txt:3-19` | Freesound attribution URLs | Attribution/reference, not stale cleanup | Sound credit file; not launch guidance. | Preserve unless licensing/attribution is audited separately. |

## Hard-Coded Local Paths

| Path/line | Reference | Classification | Suggested handling |
| --- | --- | --- | --- |
| `SVEtool.ini:6` | `C:\Program Files\Magic\` | Stale/default local Windows install path | Keep as a sample config; tell users to edit before use. |
| `SVEtool.ini:10` | `C:\Program Files\Magic\CSV\` | Stale/default local Windows install path | Keep as a sample config. |
| `PlayDeckAnalyser/PDAnalyser.ini:14-19` | `C:\Users\del_d\OneDrive\Bureau\Magic\...`, `C:\PROGRA~1\Magic\...` | Local developer paths | Treat as defaults/samples, not current repo paths. |
| `PlayDeckAnalyser/PDAnalyser.ini:42`, `:59` | Local external editor path | Local developer path | Document before changing. |
| `archive/local-helpers/shandalar_homedoom.bat:1-5` | `e:\Program Files\Magic`, `RC.exe /X 1600 /Y 900` | Machine-specific helper | Old root path was `shandalar_homedoom.bat`; archived as historical/local helper, not README guidance. |
| `src/build.pl:51-53` | `c:\magic`, `c:\magic2k`, `c:\magic2k\zips` | Stale build/deploy paths | Do not run unmodified. |
| `src/deploy.bat:1-44` | `c:\magic2k`, `c:\mingw\bin\manalink` | Stale build/deploy paths | Treat as historical deployment script. |
| `archive/debug-evidence/assertFile.txt:2` | `D:\NewMagic\sources\sidlib\lib.c` | Debug evidence | Old root path was `assertFile.txt`; archived as debug evidence. |
| `archive/debug-evidence/ML_Debug.txt:3-35` | `D:\Newmagic\FamiliarWS\...` | Debug evidence | Old root path was `ML_Debug.txt`; archived as debug evidence. |

## Current README Replacements

| Old idea | Replacement |
| --- | --- |
| "Run `Shandalar.exe` from the folder" | Run from `Program/` and test both `Shandalar.exe` and `Magic.exe`. |
| "Install latest runtime packages" | Inspect missing DLLs first; use x86 packages only when evidence points to them. |
| Old forum/download links as primary path | Local docs and local files first; historical links stay in `docs/stale-references.md`. |
