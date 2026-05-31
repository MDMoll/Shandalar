# Reorganization Decision Log

Date: 2026-05-30

Scope: limited reorganization only. Runtime assets, launch targets, source
snapshots, mods, decks, art, and important root-vs-Program mirrors were left in
place.

## Pre-Move Status

Before moving files, `git status --short --untracked-files=all` showed only the
staged case-only README rename and the new documentation files:

```text
RM readme.md -> README.md
?? AGENTS.md
?? docs/architecture.md
?? docs/building.md
?? docs/cleanup-audit.md
?? docs/command-line.md
?? docs/crossover-macos.md
?? docs/file-inventory.md
?? docs/magic-exe.md
?? docs/running.md
?? docs/runtime-dependencies.md
?? docs/stale-references.md
```

## Moves Performed

All moved files were tracked and were moved with `git mv`.

| Original path | New path | Rationale |
| --- | --- | --- |
| `Duel.GID` | `archive/generated-local/Duel.GID` | Windows Help generated bookmark/index file. |
| `shandalar_dll.log` | `archive/generated-local/shandalar_dll.log` | Runtime/debug log-like CSV text. |
| `ML_Debug.txt` | `archive/debug-evidence/ML_Debug.txt` | Debug text with old local source paths. |
| `assertFile.txt` | `archive/debug-evidence/assertFile.txt` | Assertion/debug text with old local source path. |
| `README (2).txt` | `archive/historical-docs/README (2).txt` | Historical note, not current entrypoint. |
| `Readme13.txt` | `archive/historical-docs/Readme13.txt` | Historical vendor/update readme. |
| `Readme132.txt` | `archive/historical-docs/Readme132.txt` | Historical vendor/update readme. |
| `Readme201.txt` | `archive/historical-docs/Readme201.txt` | Historical Manalink update note. |
| `Gatheringnet.url` | `archive/historical-links/Gatheringnet.url` | Old InternetShortcut file. |
| `Microprose.url` | `archive/historical-links/Microprose.url` | Old InternetShortcut file. |
| `shandalar_homedoom.bat` | `archive/local-helpers/shandalar_homedoom.bat` | Machine-specific helper with hard-coded path. |
| `Rogues_Org_BAK.csv` | `archive/backups/Rogues_Org_BAK.csv` | Backup-named data file; not referenced by local text search. |

## Move Command

```sh
mkdir -p archive/generated-local archive/debug-evidence archive/historical-docs archive/historical-links archive/local-helpers archive/backups
git mv Duel.GID shandalar_dll.log archive/generated-local/
git mv ML_Debug.txt assertFile.txt archive/debug-evidence/
git mv 'README (2).txt' Readme13.txt Readme132.txt Readme201.txt archive/historical-docs/
git mv Gatheringnet.url Microprose.url archive/historical-links/
git mv shandalar_homedoom.bat archive/local-helpers/
git mv Rogues_Org_BAK.csv archive/backups/
```

The first sandboxed `git mv` attempt could not create `.git/index.lock`, so the
same `git mv` command was rerun with approval to update the repository index.

## What Was Not Moved

`Program/`, root `Shandalar.exe`, root `Magic.exe`, root/Program DLLs,
`Cards.dat`, `Rogues.csv`, `SVEtool.ini`, `README - ManalinkEx.txt`,
`Manalink3/`, `Mods/`, `CardArt*`, `Decks*`, `src/`, and `magic_updater/` were
left in place.

## Verification Results

Final verification was run after the docs were updated.

| Check | Result |
| --- | --- |
| `git status --short --untracked-files=all` | Shows README case rename, the approved archive renames, and untracked docs/archive README files. |
| `git diff --stat` | Shows the unstaged README content update. The approved archive moves are staged by `git mv`, and new docs are untracked, so they are visible in `git status` rather than this plain diff. |
| `find archive -maxdepth 2 -type f | sort` | Lists exactly the archived files and `archive/README.md`. |
| `rg` for moved filenames across `README.md`, `AGENTS.md`, `docs`, `archive` | Moved names remain discoverable through docs and archive paths. |
| ASCII check over `README.md`, `AGENTS.md`, `docs`, `archive` | No non-ASCII text was found after the final pass. |
