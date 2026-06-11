# Source Map

This map separates active runtime surfaces from source snapshots and helper
tooling. Evidence labels follow the repo convention: `verified` means local
commands or file inspection were run in this checkout; `inferred` means the
role follows from names, strings, imports, or scripts; `not tested` means no
runtime or build proof was produced in this audit.

Generated inventory:
[generated/code-audit/source-files.tsv](generated/code-audit/source-files.tsv)
[generated/code-audit/language-summary.tsv](generated/code-audit/language-summary.tsv),
[generated/code-audit/source-snapshot-parity.tsv](generated/code-audit/source-snapshot-parity.tsv),
and
[generated/code-audit/source-snapshot-parity-summary.txt](generated/code-audit/source-snapshot-parity-summary.txt).

## Language Summary

| Language | Files | Lines | Evidence |
| --- | ---: | ---: | --- |
| C | 300 | 762401 | verified by `tools/audit_codebase.py` |
| C/C++ headers | 22 | 44775 | verified |
| C++ | 6 | 8957 | verified |
| Assembly | 45 | 45361 | verified |
| Perl | 113 | 14459 | verified |
| Shell | 21 | 3811 | verified |
| Batch/CMD | 6 | 1866 | verified |
| Python | 1 | 430 | verified |
| Makefiles | 10 | 319 | verified |
| Module definitions | 17 | 147 | verified |
| Resources | 1 | 154 | verified |

## Area Map

| Area | Language | Purpose | Runtime Impact | Build Status | Notes |
| --- | --- | --- | --- | --- | --- |
| `src/` | C, C++, ASM, Perl, make | Main source snapshot and binary patch scripts. | runtime-adjacent; not proven to rebuild shipped files | dry-run reaches build plan | `src/card_id.h` is mirrored with `Program/src/card_id.h`; the current generated tail follows root `magic_updater/Manalink.csv`. Full rebuild provenance is still unresolved. |
| `Program/src/` | C, ASM, make, Perl | Program-side source snapshot. | runtime-adjacent | dry-run reaches further | `Program/src/card_id.h` exists; source differs from top-level `src/`, though current mirrored safety edits are parity-checked. |
| `src/patches/` and `Program/src/patches/` | Perl, C, make | In-place binary patch tooling. | high, mutates binaries when run | not executed | Treat as risky until target hash, bytes, backup, and rollback are documented. |
| `magic_updater/` and `Program/magic_updater/` | Perl, CSV | Card data conversion/update tooling. | tooling and data-adjacent | not tested | The root `magic_updater/Manalink.csv` matches the active card-data namespace used for `card_id.h`; `Program/magic_updater/Manalink.csv` is older and should not be treated as authoritative without a separate decision. |
| `tools/` | Shell, Python | Repo audit, verification, scan, package, source-parity, and handoff helpers. | tooling | verified by share checks in prior work; this audit added read-only helpers | `tools/audit_codebase.py`, `tools/check-build-prereqs.sh`, and `tools/check-source-snapshot-parity.sh` are read-only by default. |
| `PlayDeckAnalyser/` | EXE plus config/data | Separate deck analysis utility. | tooling/runtime-adjacent | not tested | Root launcher can start this helper. |
| Root batch/CMD files | Batch | Launch/help/mod entry points. | runtime-adjacent | not tested in this pass | `Manalink_Launcher.cmd` mutates local `Program/` and `Mods/` trees. |
| `Program/` non-source runtime | EXE, DLL, data, assets | Canonical active runtime bundle plus Manalink target. | runtime | not rebuilt | Do not treat as generated output. |
| `Manalink3/` | Mixed package snapshot | Historical self-contained package evidence. | historical/runtime-like | not supported active root | Retained as evidence; duplicate archives were already removed from `Manalink3/Mods/`. |
| `Mods/` | Archives, assets, utilities | Canonical mod archive/staging tree. | runtime-adjacent | not applicable | Root launcher enumerates local `Mods/`. |

## Important Source Provenance Findings

| Finding | Evidence | Status |
| --- | --- | --- |
| `src/` and `Program/src/` are not interchangeable. | `source-snapshot-parity-summary.txt` records 322 tracked relative paths: 77 same, 145 different, 0 missing in `src`, and 100 missing in `Program/src`. | verified |
| Current mirrored source-safety edits have a guard but do not resolve provenance. | `card_id.h`, `cards/draft.c`, and `cardartlib/src/main.cpp` exact-match checks pass; source-safety markers pass in `cards/draft.c`, `functions/targets.c`, `cardartlib/src/main.cpp`, and `drawcardlib/config.c`. | guarded |
| Source does not prove shipped-binary reproducibility. | Build dry-runs are incomplete and no full build was attempted. | verified |
| Patch scripts encode current binary-maintenance knowledge. | Patch scripts and docs reference offsets, hooks, caves, and target EXE/DLL names. | verified |
| Runtime behavior still depends on exact launch path. | Binary strings and docs show basename/runtime-folder references such as `Cards.dat`, `DuelSounds`, `FaceMaker.exe`, and `zlib.dll`. | verified |
