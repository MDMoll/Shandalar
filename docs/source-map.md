# Source Map

This map separates active runtime surfaces from source snapshots and helper
tooling. Evidence labels follow the repo convention: `verified` means local
commands or file inspection were run in this checkout; `inferred` means the
role follows from names, strings, imports, or scripts; `not tested` means no
runtime or build proof was produced in this audit.

Generated inventory:
[generated/code-audit/source-files.tsv](generated/code-audit/source-files.tsv)
and
[generated/code-audit/language-summary.tsv](generated/code-audit/language-summary.tsv).

## Language Summary

| Language | Files | Lines | Evidence |
| --- | ---: | ---: | --- |
| C | 300 | 762077 | verified by `tools/audit_codebase.py` |
| C/C++ headers | 21 | 29128 | verified |
| C++ | 6 | 8789 | verified |
| Assembly | 45 | 45361 | verified |
| Perl | 113 | 14367 | verified |
| Shell | 19 | 3396 | verified |
| Batch/CMD | 6 | 1841 | verified |
| Makefiles | 10 | 319 | verified |

## Area Map

| Area | Language | Purpose | Runtime Impact | Build Status | Notes |
| --- | --- | --- | --- | --- | --- |
| `src/` | C, C++, ASM, Perl, make | Main source snapshot and binary patch scripts. | runtime-adjacent; not proven to rebuild shipped files | partially blocked | `src/card_id.h` is absent; `make -n` stops at `functions/utility.obj`. |
| `Program/src/` | C, ASM, make, Perl | Program-side source snapshot. | runtime-adjacent | dry-run reaches further | `Program/src/card_id.h` exists; source differs from top-level `src/`. |
| `src/patches/` and `Program/src/patches/` | Perl, C, make | In-place binary patch tooling. | high, mutates binaries when run | not executed | Treat as risky until target hash, bytes, backup, and rollback are documented. |
| `magic_updater/` and `Program/magic_updater/` | Perl, CSV | Card data conversion/update tooling. | tooling and data-adjacent | not tested | Large CSV inputs; no updater command was run. |
| `tools/` | Shell, Python | Repo audit, verification, scan, package, and handoff helpers. | tooling | verified by share checks in prior work; this audit added one helper | New `tools/audit_codebase.py` is read-only by default. |
| `PlayDeckAnalyser/` | EXE plus config/data | Separate deck analysis utility. | tooling/runtime-adjacent | not tested | Root launcher can start this helper. |
| Root batch/CMD files | Batch | Launch/help/mod entry points. | runtime-adjacent | not tested in this pass | `Manalink_Launcher.cmd` mutates local `Program/` and `Mods/` trees. |
| `Program/` non-source runtime | EXE, DLL, data, assets | Canonical active runtime bundle plus Manalink target. | runtime | not rebuilt | Do not treat as generated output. |
| `Manalink3/` | Mixed package snapshot | Historical self-contained package evidence. | historical/runtime-like | not supported active root | Retained as evidence; duplicate archives were already removed from `Manalink3/Mods/`. |
| `Mods/` | Archives, assets, utilities | Canonical mod archive/staging tree. | runtime-adjacent | not applicable | Root launcher enumerates local `Mods/`. |

## Important Source Provenance Findings

| Finding | Evidence | Status |
| --- | --- | --- |
| `src/` and `Program/src/` are not interchangeable. | `diff -qr src Program/src` reports many differing files; only `Program/src/card_id.h` is tracked. | verified |
| Source does not prove shipped-binary reproducibility. | Build dry-runs are incomplete and no full build was attempted. | verified |
| Patch scripts encode current binary-maintenance knowledge. | Patch scripts and docs reference offsets, hooks, caves, and target EXE/DLL names. | verified |
| Runtime behavior still depends on exact launch path. | Binary strings and docs show basename/runtime-folder references such as `Cards.dat`, `DuelSounds`, `FaceMaker.exe`, and `zlib.dll`. | verified |
