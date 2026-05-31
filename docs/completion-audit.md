# Completion Audit

Audit date: 2026-05-31.

This audit maps the cleanup/share goal to current evidence. It is intentionally
strict: a row is not marked complete unless the current checkout contains direct
proof.

## Current Verdict

| Area | Status | Meaning |
| --- | --- | --- |
| Local maintenance branch | Ready locally | The tree is clean and `tools/verify-share-readiness.sh` passes. |
| GitHub branch push | Blocked outside repo | Local pushes fail because this environment has no GitHub HTTPS credentials and no accepted SSH key. |
| Clean public release | Not proven | Distribution rights, malware scan, and full gameplay verification are still missing. |
| More file moves | Deferred | Remaining candidates are runtime-like, save-state, art-folder, or package-tree files that need explicit approval and launch-copy testing. |

## Requirement Audit

| Requirement | Status | Current evidence | Remaining proof |
| --- | --- | --- | --- |
| New user can read README and know where to start. | Proven locally | [../README.md](../README.md), [README.md](README.md), and `tools/verify-share-readiness.sh` docs-index check. | None for local documentation. |
| CrossOver-on-macOS user has a reasonable bottle path and commands. | Partially proven | [crossover-macos.md](crossover-macos.md), [running.md](running.md), and [verified-on-this-machine.md](verified-on-this-machine.md). | Visible manual retest of patched character creation, duel stability, save/load, and the 1440x1080 desktop setup. |
| Future agents can avoid damaging important assets. | Proven locally | [../AGENTS.md](../AGENTS.md), [cleanup-audit.md](cleanup-audit.md), and [cleanup-move-plan.md](cleanup-move-plan.md). | None for guardrail docs; future moves still need approval/testing. |
| Major folders are explained. | Proven locally | [../README.md](../README.md), [architecture.md](architecture.md), and [file-inventory.md](file-inventory.md). | None for current documentation. |
| `Magic.exe` has dedicated documentation. | Proven locally | [magic-exe.md](magic-exe.md), [magic-vs-shandalar-runtime.md](magic-vs-shandalar-runtime.md), and bug notes under [bugs/](bugs/). | Visible root-vs-`Program/` behavior comparison is still pending. |
| `Shandalar.exe --help` and command-line modes are documented or marked unverified. | Proven locally | [command-line.md](command-line.md) records attempted commands and marks `--help` capture as missing. | Capture visible help/dialog/log output on Windows or CrossOver. |
| Runtime dependency guidance is based on binary inspection. | Proven locally | [runtime-dependencies.md](runtime-dependencies.md) and [verified-on-this-machine.md](verified-on-this-machine.md). | Native Windows dependency behavior is still untested. |
| Cleanup candidates are listed with evidence and confidence, nothing deleted. | Proven for current audit scope | [cleanup-audit.md](cleanup-audit.md), [cleanup-move-plan.md](cleanup-move-plan.md), [duplicate-audit.md](duplicate-audit.md), and [save-state.md](save-state.md). | Approved launch-copy tests before moving save-state, art-folder, package-tree, or duplicate runtime-like files. |
| Stale references are identified with path/line evidence where possible. | Proven locally | [stale-references.md](stale-references.md). | Live URL reachability only if requested. |
| Generated docs distinguish verified facts from inferences. | Proven locally | Root README evidence-language note plus `Verified on this machine` and `Needs testing` sections across docs. | Keep using that convention for new investigations. |
| Branch can be shared in git. | Ready locally, push blocked | Clean tree, local commits on `codex/shandalar-crossover-updates`, and verifier pass. | Push from an authenticated GitHub environment. |
| Something that works. | Partially proven | Static binary hashes/patch bytes pass; CrossOver smoke evidence passed earlier crash points. | Manual gameplay retest is still required before claiming the game works end to end. |

## Do Not Move Yet

| Path family | Why |
| --- | --- |
| `CardArtNew/Thumbs.db` | Generated-looking, but inside a runtime-like art folder; needs explicit approval. |
| `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, `MAGIC5`, `CSV/MAGIC*/`, `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | Save/local state needs a visible save/load test before archiving or keeping as fixtures. |
| `Program/Savedescs`, `Manalink3/Program/ScreenNames/` | Package-tree local state; do not move separately from owning package/runtime trees. |
| `MENUBAK.PIC`, `WINBAK*.PIC`, `WORLBAK1.PIC`, and matching `Program/` copies | Backup-looking names, but current evidence identifies them as legacy `.PIC` resource files. |
| `FaceMaker-Original.exe`, `FaceMaker-nores.exe`, `Program/FaceMaker-nores.exe` | Reference executables documenting the active FaceMaker patch lineage. |
| Duplicate decks, mods, art, and package snapshots | Exact hashes prove duplication, not safe removal. Choose canonical paths only after launch-copy tests. |

## Final Gates Before Marking Complete

| Gate | Evidence needed |
| --- | --- |
| Push | `git push -u origin codex/shandalar-crossover-updates` succeeds. |
| Gameplay | Visible Windows or CrossOver run confirms launch, character creation, all starting colors, same-arrow map stop, save/load, several duel turns, damage-prevention prompt, and declared-attacker undo. |
| Security | A named scanner/version records results for patched executables, DLLs, and archives. |
| Distribution | A decision is recorded: private maintenance branch, patch/docs-only public package, or rights-verified full bundle. |
| Cleanup moves | Any additional file moves have explicit approval, `git mv` history, docs updates, and launch-copy verification. |
