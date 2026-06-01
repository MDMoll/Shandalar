# Install Roots

This repo has overlapping install/app layouts. Cleanup decisions should start
at the install-root level, not at isolated duplicate files.

## Policy

| Rule | Meaning |
| --- | --- |
| The repository root plus top-level `Program/` is the canonical active Shandalar layout. | Keep root launch targets, root-adjacent data, and `Program/` runtime assets. |
| Top-level `Mods/` is the canonical active mod archive/staging layout. | Keep archive copies used by the root launcher. |
| `Manalink3/` is historical/package evidence, not a supported active install root in this pass. | Its local launcher proves that package was internally coherent, but not that duplicate archives must remain in two places. |
| Runtime assets remain protected. | Do not remove active `Program/` assets, executables, DLLs, card art, `.pic`, `.spr`, `.dat`, sound, or video files during this pass. |

Generated TSV evidence:
[generated/install-root-inventory.tsv](generated/install-root-inventory.tsv) and
[generated/install-root-decision.tsv](generated/install-root-decision.tsv).

## Root Decisions

| Root | Launchers | Local folders used | Evidence | Keep/Archive/Remove | Reason |
| --- | --- | --- | --- | --- | --- |
| `.` | `Shandalar.exe`; `Magic.exe`; `FaceMaker.exe`; `Manalink_Launcher.cmd`; `Deck.exe` | `Program/`; `Mods/`; top-level card art, sounds, decks, sprite/resource folders, source/tools/docs | README and AGENTS default launch notes identify root `Shandalar.exe` as the current CrossOver `MTG` launch surface; root launcher sets paths relative to `%~dp0`. | Keep | Canonical active repo/install root. |
| `Program/` | `Program/Shandalar.exe`; `Program/Magic.exe`; `Program/FaceMaker.exe`; `Program/Deck.exe` | Program-local runtime assets, DLLs, data, decks, sounds, art, source/updater snapshots | Root launcher changes to `%mlDir%` and starts `Magic.exe`; README treats `Program/Magic.exe` as first-class. | Keep | Canonical active runtime bundle; no runtime assets removed. |
| `Mods/` | Root `Manalink_Launcher.cmd` | `Art/`; `Rogues/`; `Sound/`; `PlayDeck/`; `Util/` | Root launcher sets `modDir=%~dp0Mods` and enumerates archives under local mod folders. | Keep | Canonical active mod archive/staging tree. |
| `Manalink3/` | `Manalink3/Manalink_Launcher.cmd`; `Manalink3/Program/Magic.exe`; `Manalink3/Program/FaceMaker.exe`; analyzer helper | `Program/`; `Mods/`; `PlayDeckAnalyser/` | Launcher is byte-identical to the root launcher and internally coherent, but no active root/CrossOver path depends on `Manalink3/`. | Archive | Historical/unsupported package root retained for evidence; exact duplicate archives under `Manalink3/Mods/` were removed in favor of top-level `Mods/`. |
| `src/` | `src/make.exe`; `src/yasm.exe`; build/deploy scripts | Source and patch folders | Existing build docs warn that source does not yet prove an end-to-end rebuild of shipped binaries. | Keep | Tooling/source root, outside install-root archive cleanup. |
| `magic_updater/` | `magic_updater/Magic_updater.bat`; `magic_updater/ct2exe.exe` | Updater data and scripts | File inventory treats updater snapshots separately from runtime. | Keep | Tooling root; no path-purpose proof for removal. |
| `PlayDeckAnalyser/` | `PlayDeckAnalyser/PlayDeckAnalyser.exe`; `PlayDeckAnalyser/MetaPad.exe` | `CSV/`; `Lists/` | Root launcher menu option starts the top-level analyzer. | Keep | Utility root used by the active root launcher. |
| `Facemaker/` | `Facemaker/Facemaker.exe` | Folder-local helper/support files | Runtime comparison docs treat this as comparison evidence, not the active helper. | Keep | Historical comparison evidence. |

## Current Result

This pass removed exact duplicate archive files only from the unsupported
`Manalink3/Mods/` package root. The top-level `Mods/` copies remain canonical.
No active runtime assets were removed.
