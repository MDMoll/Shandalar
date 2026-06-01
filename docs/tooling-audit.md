# Tooling Audit

Generated evidence:
[generated/code-audit/script-tooling-risks.tsv](generated/code-audit/script-tooling-risks.tsv).

| Script | Purpose | Problem | Evidence | Suggested Fix | Safe to Fix Now? |
| --- | --- | --- | --- | --- | --- |
| `tools/audit_codebase.py` | Read-only inventory and regex audit helper. | New helper; regex-only, not semantic analysis. | Created in this audit and run successfully. | Keep scope small; add focused-scope option later. | yes |
| `src/build.pl` | Add a card function to `ManalinkEh.asm`, run make, copy DLL. | Mutates source and copies to `c:\magic2k` without dry-run. | Static inspection lines 1-53. | Add `--dry-run`, `--out`, and explicit `--copy-to`. | not in this pass |
| `src/deploy.bat` | Historical packaging copy script. | Hardcoded destructive deletes/copies under `c:\magic2k`. | Static inspection. | Mark historical; do not run from repo. | docs only |
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
