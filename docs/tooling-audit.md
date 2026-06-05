# Tooling Audit

Generated evidence:
[generated/code-audit/script-tooling-risks.tsv](generated/code-audit/script-tooling-risks.tsv).

| Script | Purpose | Problem | Evidence | Suggested Fix | Safe to Fix Now? |
| --- | --- | --- | --- | --- | --- |
| `tools/audit_codebase.py` | Read-only inventory and regex audit helper. | Regex-only, not semantic analysis; broad scans can still be requested. | `--scan-scope focused` is now the default; `--scan-scope all` preserves the original broad report mode; it now also writes `damage-prevention-handlers.tsv`. | Add more focused named scopes later if useful. | yes |
| `tools/check-build-prereqs.sh` | Read-only source/build preflight. | Reports blockers but does not repair them. | Added after the audit; `--report-only` captures missing local toolchain/header state. | Use before any build or historical helper run. | yes |
| `tools/check-source-snapshot-parity.sh` | Read-only `src/` versus `Program/src/` parity checker. | Reports source divergence but does not choose the authoritative snapshot. | Records 80 matching paths, 142 differing paths, and 100 paths missing from `Program/src`; exact-match files and current safety markers pass. | Run after mirrored source-safety edits and before source-to-runtime claims. | yes |
| `src/build.pl` | Add a card function to `ManalinkEh.asm`, run make, optionally copy DLL. | Still edits source when intentionally run with a card name, but no longer silently copies to `c:\magic2k`. | Added `--dry-run`, `--out-dir`, `--copy-to`, and `--legacy-copy-targets`; legacy copies are opt-in. | Use `--dry-run` first; use explicit copy targets only in a disposable/build environment. | yes |
| `src/deploy.bat` | Historical packaging copy script. | Confirmed mode still uses hardcoded destructive deletes/copies under `c:\magic2k`, but casual runs now exit before mutation. | Added `--confirmed-c-magic2k-deploy` guard. | Run confirmed mode only in a prepared Windows packaging copy. | yes |
| `Manalink_Launcher.cmd` | Launch Manalink and install mods. | Mutates local `Program/`/`Mods/`; deletes draft decks; enumerates local archives. | Static inspection with `%~dp0`, `DEL`, `RD`, `COPY`. | Use only in disposable-copy tests for cleanup work. | docs only |
| `Manalink3/Manalink_Launcher.cmd` | Historical package-local launcher. | Internally coherent but unsupported active root. | install-root docs. | Keep historical; do not restore duplicate archives just for it. | docs only |
| `src/patches/*.pl` | Binary patch scripts. | Mutate EXE/DLL targets in-place. | Patch inventory and AGENTS guardrails. | Require hash/backup/rollback/test plan. | no |
| `magic_updater/*.pl` | Card data conversion/update. | Not tested; likely historical assumptions. | inventory only. | Add usage/dry-run docs before running. | not yet |
| `tools/verify-*.sh` | Repo verification gates. | Mostly healthy; some scans depend on exact tracked target hashes. | share-readiness passed in previous work. | Keep local evidence TSV refresh workflow documented. | yes |

## Safe Fixes Applied

| Fix | Path | Why Safe | Test Run |
| --- | --- | --- | --- |
| Added read-only audit helper. | `tools/audit_codebase.py` | Standard library only; writes only under `docs/generated/code-audit/`; does not launch game or mutate runtime folders. | `python3 tools/audit_codebase.py --out docs/generated/code-audit` |
| Added generated audit reports. | `docs/generated/code-audit/` | Documentation/evidence only. | File counts and TSV summaries inspected. |
| Added focused scan mode. | `tools/audit_codebase.py` | Keeps full inventory while reducing default grep noise. | `python3 tools/audit_codebase.py --help`; regenerated reports. |
| Added damage-prevention handler inventory. | `tools/audit_codebase.py`; `docs/generated/code-audit/damage-prevention-handlers.tsv` | Static source evidence only; does not launch the game or patch DLLs. | `python3 -m py_compile tools/audit_codebase.py`; `python3 tools/audit_codebase.py --out docs/generated/code-audit --scan-scope focused`. |
| Added build preflight. | `tools/check-build-prereqs.sh` | Read-only; does not build or copy. | `tools/check-build-prereqs.sh --report-only`. |
| Added source snapshot parity checker. | `tools/check-source-snapshot-parity.sh` | Read-only; writes optional reports only under a requested report directory. | `bash -n tools/check-source-snapshot-parity.sh`; `tools/check-source-snapshot-parity.sh`; `tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only`. |
| Hardened historical build helper. | `src/build.pl` | Removes implicit external copy; adds dry-run and explicit output flags. | `perl -c src/build.pl`; `perl src/build.pl --help`; `(cd src && perl build.pl --dry-run black_lotus)`. |
| Guarded historical deploy helper. | `src/deploy.bat` | No-argument and help paths exit before destructive `c:\magic2k` packaging commands. | Static inspection of guard and confirmation argument. |
