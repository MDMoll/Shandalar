# Runtime Path Audit

This audit focuses on path and working-directory risk. It did not launch the
game.

Generated evidence:
[generated/code-audit/runtime-file-references.tsv](generated/code-audit/runtime-file-references.tsv),
[generated/code-audit/hardcoded-paths.tsv](generated/code-audit/hardcoded-paths.tsv),
and
[generated/code-audit/binary-runtime-strings.txt](generated/code-audit/binary-runtime-strings.txt).

## Canonical Layout

| Layout | Status | Evidence |
| --- | --- | --- |
| Repo root plus top-level `Program/` | verified canonical active layout | README, AGENTS, install-root docs, launcher path behavior |
| Top-level `Mods/` | verified canonical mod archive/staging root | `Manalink_Launcher.cmd` uses `%~dp0Mods` |
| `Manalink3/` | verified historical/unsupported active root | install-root cleanup decision |
| Direct `Program/Shandalar.exe` in CrossOver `MTG` | verified problematic | docs record missing `Program\zlib.dll` load failure |

## Runtime String Findings

| Reference | Evidence | Interpretation |
| --- | --- | --- |
| `Cards.dat` | strings in `Magic.exe`, `DeckDLL.dll`, `CardArtLib.dll` | basename lookup; working directory matters |
| `DuelSounds`, `Sound` | strings in `Magic.exe`, `DeckDLL.dll`, `cdtools.dll` | relative runtime folders |
| `%s\FaceMaker.exe` | strings in root and Program `Magic.exe` families | helper path is computed from a base directory |
| `FaceMaker-nores.exe` | string in root `ManalinkEx.dll`/related runtime evidence | preserve no-resolution helper until handoff is fully understood |
| `zlib.dll` | strings in image/statwin-related DLLs | adjacent DLL layout matters |
| `D:\NewMagic\...` | FaceMaker/source assertion strings | likely compile-time/debug path unless file tracing proves access |

## Path Risks

| Risk | Status | Evidence | Next Step |
| --- | --- | --- | --- |
| Wrong working directory for `Cards.dat`/art/sound | verified risk | basename and relative strings | launch exact paths from their intended folders only |
| Root vs `Program` `Magic.exe` confusion | verified risk | different SHA-256 and launch paths | keep separate test rows and patch verification |
| `Program/zlib.dll` absence | verified risk in `MTG` | existing logged failure | test fixed layout only in disposable copy |
| Launcher mutates local `Program/` and `Mods/` | verified | `Manalink_Launcher.cmd` uses `%~dp0` and deletes/copies local files | use cleanup test copies |
| Hardcoded historical `c:\magic2k` scripts | verified | `src/build.pl`, `src/deploy.bat` | mark historical until dry-run guards exist |

## Layout Questions Still Open

| Question | Evidence Needed |
| --- | --- |
| Can `Program/Shandalar.exe` be made a supported active path? | Add/copy missing adjacent DLLs in a disposable copy and run exact-path launch tests. |
| Which `Magic.exe` should be canonical? | Visible root and Program launch comparison with hashes recorded. |
| Are any remaining `Manalink3/` files required by active root? | File tracing or launcher/source references from root layout. |
| Which FaceMaker helper name is actually spawned in all new-game paths? | Manual character-creation test plus process/log evidence. |
