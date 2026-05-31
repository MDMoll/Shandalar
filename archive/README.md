# Archive

This folder preserves generated, debug, local-helper, backup, and historical
files that used to sit at the repository root.

These files were moved with `git mv` during the limited reorganization pass on
2026-05-30. They are not deleted, and they are not treated as disposable. Keep
them versioned unless a later audit proves a specific file can be removed.

## Layout

| Folder | Contents |
| --- | --- |
| `generated-local/` | Generated or local runtime/debug output. |
| `debug-evidence/` | Text files with old assertion/debug paths. |
| `historical-docs/` | Old readmes and update notes kept as source material. |
| `historical-links/` | Old InternetShortcut files. |
| `local-helpers/` | Machine-specific helper scripts. |
| `backups/` | Backup-named data files. |

Do not move launch targets, DLLs, data files, art, decks, mods, source
snapshots, or runtime folders here without explicit approval and a launch-copy
test plan.
