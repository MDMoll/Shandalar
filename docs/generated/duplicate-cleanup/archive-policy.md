# Archive Duplicate Policy

This policy applies to duplicate archive files that appear under more than one install root.

## Current Canonical Policy

The active root install layout is the repository root plus top-level `Program/` and top-level `Mods/`. `Manalink3/` is retained as historical/package evidence, but it is not a supported active install root in this pass.

A launcher enumerating its local `Mods` folder proves that a package root is internally coherent. It does not prove the cleaned repo must keep duplicate archives under that package root.

## Applied Cleanup

Exact duplicate archive files under `Manalink3/Mods/Art/` and `Manalink3/Mods/Rogues/` were removed after the install-root decision selected top-level `Mods/` as canonical. The kept copies remain under `Mods/Art/` and `Mods/Rogues/`.

No executable-adjacent files, DLLs, card art stores, sound/video assets, `.dat`, `.pic`, `.spr`, or active `Program/` runtime files were removed by this policy pass.
