# Baseline

Date: 2026-05-30

| Item | Value |
| --- | --- |
| Working directory | `/Users/mdmoll/Shandalar/Shandalar` |
| Git top-level | `/Users/mdmoll/Shandalar/Shandalar` |
| Git HEAD | `868a7c5b0cc373c531197edd0a130c4d36cd7a1d` |
| Host OS used for static analysis | macOS 15.7.5, Darwin 24.6.0, arm64 |
| Static tools found | `/opt/homebrew/bin/rg`, `/usr/bin/file`, `/usr/bin/strings`, `/usr/bin/objdump`, `/usr/bin/otool` |
| Static tools not found in `command -v` check | `r2`, `rabin2` |
| Native Windows runtime testing | Not performed in this pass. |
| Wine/CrossOver runtime testing | Not performed in this pass; user-provided crash log was analyzed. |

## Pre-investigation Git Status

```text
RM readme.md -> README.md
R  Rogues_Org_BAK.csv -> archive/backups/Rogues_Org_BAK.csv
R  ML_Debug.txt -> archive/debug-evidence/ML_Debug.txt
R  assertFile.txt -> archive/debug-evidence/assertFile.txt
R  Duel.GID -> archive/generated-local/Duel.GID
R  shandalar_dll.log -> archive/generated-local/shandalar_dll.log
R  "README (2).txt" -> "archive/historical-docs/README (2).txt"
R  Readme13.txt -> archive/historical-docs/Readme13.txt
R  Readme132.txt -> archive/historical-docs/Readme132.txt
R  Readme201.txt -> archive/historical-docs/Readme201.txt
R  Gatheringnet.url -> archive/historical-links/Gatheringnet.url
R  Microprose.url -> archive/historical-links/Microprose.url
R  shandalar_homedoom.bat -> archive/local-helpers/shandalar_homedoom.bat
?? AGENTS.md
?? archive/README.md
?? docs/
```
