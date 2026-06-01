# Archive Duplicate Policy

This policy applies to the verified duplicate cleanup pass.

## Default Rule

When an archive duplicate exists under both `Mods/` and `Manalink3/Mods/`, the default canonical location would be `Mods/` and the removable duplicate would be under `Manalink3/Mods/`.

## Local Evidence Override

That default is not applied in this checkout because `Manalink3/` has strong package-layout evidence:

| Evidence | Meaning |
| --- | --- |
| `Manalink3/Manalink_Launcher.cmd` exists and matches the root launcher shape. | The tree can be launched from inside `Manalink3/`. |
| The launcher sets `mlDir=%~dp0Program` and `modDir=%~dp0Mods`. | `Manalink3/Mods/` is package-local, not a stray mirror by default. |
| The launcher counts `*.7z` and `*.zip` under local `Art`, `Sound`, and `Rogues` folders. | Removing local duplicate archives would change the mod menu inventory for that package. |
| `Manalink3/` contains its own `Program/`, `Mods/Util/`, `PlayDeckAnalyser/`, and docs. | The directory is shaped like a self-contained distribution snapshot. |

## Current Decision

Archive duplicate pairs under `Mods/` and `Manalink3/Mods/` are classified as `protected-package-layout`, not `safe-remove-after-canonical-policy`.

A future cleanup may apply the default keep-`Mods/` policy only after the user explicitly decides that `Manalink3/` no longer needs to remain self-contained and a disposable mod-menu/listing test records the effect.
