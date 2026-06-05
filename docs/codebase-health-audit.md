# Codebase Health Audit

Audit branch: `codex/deep-codebase-health-audit`.

This started as a static/source/tooling pass. It did not launch Shandalar, did
not run Wine/CrossOver, and did not rebuild binaries. A later bounded runtime
layout fix added `Program/zlib.dll` as a byte-for-byte copy of root `zlib.dll`;
later Program-path `drawcardlib.dll` and config/font findings were addressed by
copying the required Program CardArt, `DuelArt`, `Manalink.ini`, and hardcoded
font files observed so far from preserved local evidence. No patched executable
or patched DLL was replaced.

Generated evidence lives under
[generated/code-audit/](generated/code-audit/). The most useful entry points are
`potential-bugs.tsv`, `build-probe-results.tsv`, `source-files.tsv`,
`language-summary.tsv`, `grep-hazard-results.tsv`, `runtime-file-references.tsv`,
`binary-runtime-strings.txt`, `script-tooling-risks.tsv`,
`source-snapshot-parity.tsv`, `source-snapshot-parity-summary.txt`, and
`damage-prevention-handlers.tsv`.

## Existing Documentation Findings

| Doc | Relevant Claim | Verified? | Notes |
| --- | --- | --- | --- |
| `README.md` | Root plus `Program/` is the practical launch surface; `Program/Shandalar.exe` was risky in `MTG` because `Program/zlib.dll`, Program config/font files, Program CardArt assets, and older Program card-data files were missing or mismatched, and now needs visible exact-path retesting after the repo and copied-install layout fixes. | verified | Confirmed by existing docs, runtime string/path evidence, matching root/Program `zlib.dll` hashes, Program adjacent config/font hashes through `DuelArt/Planeswalker.dat` and six `TT*.ttf` files, Program CardArt asset hashes in the repo and local `MTG` copy, matching Program card-data hashes, and the latest bounded log opening the Program card-data trio, `shandalar.dll`, and `Shandalar.ini` without the earlier fatal strings. |
| `AGENTS.md` | Patched binaries must not be casually replaced. | verified | Patch sites are documented and included in the patch risk register. |
| `docs/architecture.md` | `src/` is source/patch tooling, not an end-to-end rebuild proof. | verified | Build dry-runs support this. |
| `docs/runtime-testing-policy.md` | GUI testing should stay bounded. | verified | This audit did no GUI testing. |
| `docs/duplicate-audit.md` | Duplicate cleanup is install-root/layout-sensitive. | verified | No cleanup was performed here. |
| `docs/troubleshooting.md` | Start-color, healer freeze, and attacker undo fixes still need manual gameplay verification. | verified | Manual gameplay gate remains incomplete. |
| `docs/building.md` | Builds are not proven and have missing tool/header blockers. | verified | Current dry-runs reproduce/extend the blockers. |
| `docs/bugs/*` | Known patch areas have evidence but need gameplay retests. | verified | Included as high-risk patch areas. |

## Highest-Value Findings

| ID | Finding | Evidence Label | Why It Matters |
| --- | --- | --- | --- |
| B001 | Top-level `src` now dry-runs to a `ManalinkEh.dll` build plan, but real builds remain blocked. | mitigated | Source fixes still cannot be assumed to affect runtime until missing tools and provenance are resolved. |
| B002 | Required Windows DLL tools are unavailable locally. | verified | Full build is blocked. |
| B003 | `src` and `Program/src` still diverge, but current mirrored source-safety fixes now have parity checks. | guarded | There is still no single obvious source of truth or source-to-runtime proof. |
| B006 | `CardArtLib` loader now checks file/read/allocation/name bounds. | mitigated source-level fix | Reduces a plausible crash path if data is missing or malformed; runtime proof still needs build/test support. |
| B010 | Draft custom-set parser now uses bounded line parsing. | mitigated source-level fix | Reduces a plausible data-driven crash/corruption path; runtime proof still needs build/test support. |
| B011 | Draft output opens now fail closed, though output paths remain runtime-local. | partially mitigated | Reduces crash risk when draft debug/deck output files cannot be opened; working-directory clutter still needs a draft-mode copy test before relocation. |
| B012 | Direct `Program/Shandalar.exe` adjacent layout gaps are mitigated through the known card-data failures. | partially mitigated | Static copied-install hashes and bounded logs now cover zlib, Program CardArt, adjacent config/font files, the Program card-data trio, `shandalar.dll`, and `Shandalar.ini`; the exact Program path still needs visible retesting before it becomes a supported target. |
| B015 | Attacker undo patch may not reverse attack costs/triggers. | needs manual test | Feature needs gameplay boundaries before broader claims. |
| B016 | Damage-prevention prompt freezes still need manual repro proof. | runtime mitigated | Source snapshots and current runtime DLLs now guard both the Samite-family handler and the generic activated `GAA_DAMAGE_PREVENTION*` helper path; visible duel retesting is still required before claiming the class fixed. |
| B023 | AI speculation restored one raw-mana row from uninitialized stack data. | runtime mitigated | Source snapshots, current repo DLLs, and local `MTG` copied-install DLLs now save both players' `raw_mana_available` rows before temporarily replacing the opponent row; visible opponent-turn retesting is still required. |

## Tooling and Script Risks

| Script | Purpose | Problem | Evidence | Suggested Fix | Safe to Fix Now? |
| --- | --- | --- | --- | --- | --- |
| `src/build.pl` | Build/copy `ManalinkEh.dll`. | Still mutates `ManalinkEh.asm` when intentionally invoked with a card name, but no longer copies to `c:\magic2k` by default. | static inspection and dry-run | Use `--dry-run`; copy only with explicit `--out-dir`, `--copy-to`, or `--legacy-copy-targets`. | yes |
| `src/deploy.bat` | Packaging script. | Confirmed mode still has hardcoded destructive Windows paths, but accidental no-argument runs now stop before mutation. | static inspection | Use `--confirmed-c-magic2k-deploy` only in a prepared Windows packaging copy. | yes |
| `Manalink_Launcher.cmd` | Runtime launcher/mod installer. | Mutates local mod/runtime folders. | static inspection | Use in disposable copies for cleanup tests. | docs only |
| `src/patches/*.pl` | Binary patch tools. | In-place EXE/DLL mutation. | patch inventory | Require hash/backup/rollback/test plan. | no |
| `tools/audit_codebase.py` | Audit helper. | Regex-only; can be noisy. | generated reports | Add focused scope later if useful. | yes |

## Safe Fixes Applied

| Fix | Path | Why Safe | Test Run |
| --- | --- | --- | --- |
| Added read-only audit helper. | `tools/audit_codebase.py` | Writes only under `docs/generated/code-audit/`; standard library only. | `python3 tools/audit_codebase.py --out docs/generated/code-audit` |
| Added build/static/path/patch/generated audit reports. | `docs/generated/code-audit/` | Evidence only; no runtime files changed. | TSV/text reports inspected. |
| Added health audit docs. | `docs/codebase-health-audit.md` and companion docs | Documentation only. | Markdown/link checks to run before commit. |

## Fix Pass: Build And Tooling Safety

| Fix | Path | Why Safe | Test Run |
| --- | --- | --- | --- |
| Added focused scan mode to the audit helper. | `tools/audit_codebase.py` | Keeps all-file inventory while reducing default grep-style scans to source/tooling files; full scans remain available with `--scan-scope all`. | `python3 tools/audit_codebase.py --help`; `python3 tools/audit_codebase.py --out docs/generated/code-audit`; `python3 tools/audit_codebase.py --out /private/tmp/shandalar-code-audit-all --scan-scope all` |
| Added read-only build prerequisite preflight. | `tools/check-build-prereqs.sh` | Reports missing tools/header/provenance risks without building, copying, launching, or touching runtime files. | `tools/check-build-prereqs.sh --report-only` |
| Hardened historical build helper. | `src/build.pl` | Removes implicit `c:\magic2k` copy behavior; adds `--dry-run`, `--out-dir`, `--copy-to`, and explicit `--legacy-copy-targets`. | `perl -c src/build.pl`; `perl src/build.pl --help`; `(cd src && perl build.pl --dry-run black_lotus)` |
| Guarded historical deploy helper. | `src/deploy.bat` | Prevents accidental destructive packaging runs under `c:\magic2k`; confirmed mode remains historical and hardcoded. | Static inspection of `--confirmed-c-magic2k-deploy` guard. |
| Restored top-level card id header. | `src/card_id.h` | Text source header copied from tracked `Program/src/card_id.h`; no runtime file changed. | `shasum -a 256 Program/src/card_id.h src/card_id.h`; `(cd src && make -n)` |
| Hardened Rotisserie custom-set draft parsing. | `src/cards/draft.c`; `Program/src/cards/draft.c` | Source-only parser fix mirrored across snapshots; no shipped binary changed. | `rg -n "fscanf\\([^\\n]*%\\[" src/cards/draft.c Program/src/cards/draft.c`; source dry-runs |
| Guarded draft output file handling. | `src/cards/draft.c`; `Program/src/cards/draft.c` | Source-only guard for runtime-local draft logs/deck writes; paths unchanged and no shipped binary changed. | `cmp -s src/cards/draft.c Program/src/cards/draft.c`; static grep for `open_draft_output`; parity checker |
| Hardened auto-target text parsing. | `src/functions/targets.c`; `Program/src/functions/targets.c` | Source-only parser fix mirrored across snapshots; no shipped binary changed. | `rg -n "fscanf\\([^\\n]*%\\[" src/functions/targets.c Program/src/functions/targets.c`; source dry-runs |
| Hardened CardArtLib data/art-name parsing. | `src/cardartlib/src/main.cpp`; `Program/src/cardartlib/src/main.cpp` | Source-only loader/path fix mirrored across snapshots; no shipped DLL changed. | CardArtLib make dry-runs; static inspection; syntax-only attempt blocked by missing Windows headers/local 32-bit assumptions. |
| Hardened drawcardlib config formatting. | `src/drawcardlib/config.c`; `Program/src/drawcardlib/config.c` | Source-only config/diagnostic formatting fix mirrored across snapshots; no shipped DLL changed. | Targeted static grep; drawcardlib make dry-runs; long-path/runtime proof still needs build/test support. |
| Added source snapshot parity guard. | `tools/check-source-snapshot-parity.sh`; `docs/generated/code-audit/source-snapshot-parity.tsv`; `docs/generated/code-audit/source-snapshot-parity-summary.txt` | Read-only source provenance evidence; enforces exact-match files and mirrored safety markers without flattening known historical source differences. | `bash -n tools/check-source-snapshot-parity.sh`; `tools/check-source-snapshot-parity.sh`; `tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only` |
| Added generic activated damage-prevention source guard, runtime helper guard, and inventory. | `src/functions/functions.c`; `Program/src/functions/functions.c`; `ManalinkEh.dll`; `Program/ManalinkEh.dll`; `tools/audit_codebase.py`; `tools/patch-manalink-generic-damage-prevention-guard.py`; `docs/generated/code-audit/damage-prevention-handlers.tsv` | Source and runtime helper paths now both gate generic activated `GAA_DAMAGE_PREVENTION*` prompts on `LCBP_DAMAGE_PREVENTION`; manual duel proof is still required. | `python3 -m py_compile tools/audit_codebase.py tools/patch-manalink-generic-damage-prevention-guard.py`; `python3 tools/audit_codebase.py --out docs/generated/code-audit --scan-scope focused`; `python3 tools/patch-manalink-generic-damage-prevention-guard.py` |
| Added AI decision-time source clamp and runtime cave patch. | `src/functions/ai.c`; `Program/src/functions/ai.c`; `ManalinkEh.dll`; `Program/ManalinkEh.dll`; `tools/patch-ai-decision-clamp.py`; `docs/bugs/opponent-turn-ai-decision-time.md` | Source and runtime paths now preserve `AiDecisionTime` values `1..270` and use `270` for missing, invalid, or higher values; manual opponent-turn proof is still required. | `python3 tools/patch-ai-decision-clamp.py`; `objdump` cave checks; `tools/verify-crossover-mtg-state.sh`; `ALLOW_DIRTY=1 tools/verify-share-readiness.sh` |
| Added AI raw-mana speculation snapshot source and runtime patch. | `src/functions/ai.c`; `Program/src/functions/ai.c`; `ManalinkEh.dll`; `Program/ManalinkEh.dll`; `tools/patch-ai-raw-mana-snapshot.py`; `docs/bugs/ai-raw-mana-snapshot.md` | Source, repo-runtime, and local `MTG` copied-install paths now save both players' `raw_mana_available` rows before AI speculation temporarily replaces only the opponent row. | `python3 tools/patch-ai-raw-mana-snapshot.py`; `tools/verify-crossover-mtg-state.sh`; `objdump` cave checks; `ALLOW_DIRTY=1 tools/verify-share-readiness.sh` |
| Guarded AI blocker speculation handoff. | `src/functions/ai.c`; `Program/src/functions/ai.c`; `tools/check-source-snapshot-parity.sh` | Source-only guard caps the speculative blocker loop to the fixed card-slot range and validates the blocker id before dispatching `TRIGGER_PAY_TO_BLOCK`; no shipped DLL changed. | Source diff review; source parity marker checks; source dry-runs |

## Fix Pass: Runtime Layout Guard

| Fix | Path | Why Safe | Test Run |
| --- | --- | --- | --- |
| Added adjacent Program zlib DLL. | `Program/zlib.dll`; `docs/runtime-manifest.md`; `tools/verify-share-readiness.sh` | Byte-for-byte copy of existing root `zlib.dll`; no patched executable or patched DLL was replaced. | `shasum -a 256 zlib.dll Program/zlib.dll`; `file zlib.dll Program/zlib.dll`; `git check-attr binary -- zlib.dll Program/zlib.dll`; share-readiness manifest/hash checks |
| Added Program adjacent config/font and CardArt files required by logged direct Program-path runs. | `Program/Manalink.ini`; `Program/DuelArt/Modern.dat`; `Program/DuelArt/Planeswalker.dat`; `Program/TT0530m_.ttf`; `Program/TT0127m_.ttf`; `Program/TT0085m_.ttf`; `Program/TT0298m_.ttf`; `Program/TT0299m_.ttf`; `Program/TT0300m_.ttf`; `Program/CardArt/ManaSymbols.pic`; `Program/CardArt/Expansion_Symbols.pic`; `Program/CardArt/Watermarks.pic`; `Program/CardArt/CardCounters.png`; `Program/CardArt/Modern/Triggering.png`; `Program/CardArt/Modern/CardOv_Nyx.png`; `Program/CardArt/Planeswalker/LoyaltyBase.png`; `Program/CardArt/Planeswalker/LoyaltyMinus.png`; `Program/CardArt/Planeswalker/LoyaltyPlus.png`; `Program/CardArt/Planeswalker/LoyaltyZero.png`; `docs/runtime-manifest.md`; `tools/verify-crossover-mtg-state.sh` | Copied from active root config/font files and preserved `Mods/Art/_undo/...` Program-style evidence after visible/logged missing-file findings; no patched executable or patched DLL was replaced. | `shasum -a 256` and `file` on the Program files listed in [runtime-manifest.md](runtime-manifest.md); `git check-attr binary` on the runtime binary/art/data files; local copied-install hash checks; latest bounded Program-path log opened these paths before exiting with code 1 |

## Dead, Stale, And Partial Areas

| Area | Status | Evidence | Next Step |
| --- | --- | --- | --- |
| Top-level `src` build | partial | `make -n` now reaches full plan; preflight reports missing Windows DLL tools | controlled toolchain branch |
| `Program/src` build | partial | dry-run plan exists but tools absent | controlled toolchain branch |
| `src/deploy.bat` | guarded historical | hardcoded `c:\magic2k` packaging paths behind explicit confirmation | keep documented; confirmed mode only in prepared Windows packaging copy |
| `Manalink3/` | historical package snapshot | install-root docs | keep as evidence unless user approves archive move |
| Generated reports | active evidence | `docs/generated/code-audit/` plus the source snapshot parity report | regenerate with helpers as needed |

## Recommended Next Fix Passes

| Branch | Goal | Candidate Fixes | Required Tests | Risk |
| --- | --- | --- | --- | --- |
| `codex/fix-build-blockers` | Make build status reproducible. | Header generation/copy plan, make rule fixes, output isolation. | `make -n`; isolated build if toolchain available. | moderate |
| `codex/fix-tooling-safety` | Make historical scripts harder to misuse. | Add dry-run/output flags to `src/build.pl`; docs for `deploy.bat`. | Perl syntax tests; dry-run test. | moderate |
| `codex/fix-path-handling` | Reduce launch/path confusion. | Program `zlib.dll` copy-test plan, exact launch matrix docs. | disposable-copy launches. | moderate |
| `codex/fix-gdi-error-reporting` | Improve graphics failure diagnostics. | Source-level checks around image/DIB/config loading. | build proof plus manual launch tests. | risky |
| `codex/retest-duel-freeze-patches` | Turn the current runtime freeze mitigations into gameplay proof. | Run visible tests for Femeref/Samite/Kithkin, generic damage-prevention prompts, ordinary spell casting, and opponent turns after fully quitting old CrossOver processes. | manual gameplay rows plus exact card/phase/process evidence for any remaining freeze. | moderate |
| `codex/add-static-analysis-ci` | Add optional analyzers once build scope is stable. | cppcheck/semgrep configs. | analyzer reports without runtime changes. | safe |

## Blocked Or Not Done

| Item | Reason |
| --- | --- |
| Full rebuild | Toolchain missing and source provenance unresolved. |
| Runtime/gameplay verification | Out of scope for this static pass. |
| Binary fixes | Explicitly avoided; current patches are protected. |
| Native Windows proof | Needs a Windows environment. |
| Semantic static analysis | Optional tools are not installed. |
