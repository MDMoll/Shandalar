# Codebase Health Audit

Audit branch: `codex/deep-codebase-health-audit`.

This was a static/source/tooling pass. It did not launch Shandalar, did not run
Wine/CrossOver, did not rebuild binaries, and did not modify runtime assets.

Generated evidence lives under
[generated/code-audit/](generated/code-audit/). The most useful entry points are
`potential-bugs.tsv`, `build-probe-results.tsv`, `source-files.tsv`,
`language-summary.tsv`, `grep-hazard-results.tsv`, `runtime-file-references.tsv`,
`binary-runtime-strings.txt`, and `script-tooling-risks.tsv`.

## Existing Documentation Findings

| Doc | Relevant Claim | Verified? | Notes |
| --- | --- | --- | --- |
| `README.md` | Root plus `Program/` is the practical launch surface; `Program/Shandalar.exe` is risky in `MTG` due missing `zlib.dll`. | verified | Confirmed by existing docs and runtime string/path evidence. |
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
| B001 | Top-level `src` cannot dry-run to `ManalinkEh.dll`. | verified | Source fixes cannot be assumed to affect runtime. |
| B002 | Required Windows DLL tools are unavailable locally. | verified | Full build is blocked. |
| B003 | `src` and `Program/src` diverge. | verified | There is no single obvious source of truth. |
| B006 | `CardArtLib` loader has unchecked file/read/allocation paths. | source-level suspicion only | Plausible crash path if data is missing or malformed. |
| B010 | Draft parser uses unbounded `fscanf`. | source-level suspicion only | Plausible data-driven crash/corruption path. |
| B012 | Direct `Program/Shandalar.exe` path lacks adjacent `zlib.dll` in `MTG`. | verified | User can hit loader failures by choosing the wrong path. |
| B015 | Attacker undo patch may not reverse attack costs/triggers. | needs manual test | Feature needs gameplay boundaries before broader claims. |
| B016 | Healer fix may not cover all damage-prevention handlers. | needs manual test | Related freeze class may have more cards. |

## Tooling and Script Risks

| Script | Purpose | Problem | Evidence | Suggested Fix | Safe to Fix Now? |
| --- | --- | --- | --- | --- | --- |
| `src/build.pl` | Build/copy `ManalinkEh.dll`. | Mutates `ManalinkEh.asm`, runs make, copies to `c:\magic2k`. | static inspection | Add dry-run/out-dir controls. | no |
| `src/deploy.bat` | Packaging script. | Hardcoded destructive Windows paths. | static inspection | Mark historical, do not run casually. | docs only |
| `Manalink_Launcher.cmd` | Runtime launcher/mod installer. | Mutates local mod/runtime folders. | static inspection | Use in disposable copies for cleanup tests. | docs only |
| `src/patches/*.pl` | Binary patch tools. | In-place EXE/DLL mutation. | patch inventory | Require hash/backup/rollback/test plan. | no |
| `tools/audit_codebase.py` | Audit helper. | Regex-only; can be noisy. | generated reports | Add focused scope later if useful. | yes |

## Safe Fixes Applied

| Fix | Path | Why Safe | Test Run |
| --- | --- | --- | --- |
| Added read-only audit helper. | `tools/audit_codebase.py` | Writes only under `docs/generated/code-audit/`; standard library only. | `python3 tools/audit_codebase.py --out docs/generated/code-audit` |
| Added build/static/path/patch/generated audit reports. | `docs/generated/code-audit/` | Evidence only; no runtime files changed. | TSV/text reports inspected. |
| Added health audit docs. | `docs/codebase-health-audit.md` and companion docs | Documentation only. | Markdown/link checks to run before commit. |

## Dead, Stale, And Partial Areas

| Area | Status | Evidence | Next Step |
| --- | --- | --- | --- |
| Top-level `src` build | partial | dry-run failure and missing `src/card_id.h` | build-blocker branch |
| `Program/src` build | partial | dry-run plan exists but tools absent | controlled toolchain branch |
| `src/deploy.bat` | historical | hardcoded `c:\magic2k` packaging paths | keep documented, do not run |
| `Manalink3/` | historical package snapshot | install-root docs | keep as evidence unless user approves archive move |
| Generated reports | active evidence | `docs/generated/code-audit/` | regenerate with helper as needed |

## Recommended Next Fix Passes

| Branch | Goal | Candidate Fixes | Required Tests | Risk |
| --- | --- | --- | --- | --- |
| `codex/fix-build-blockers` | Make build status reproducible. | Header generation/copy plan, make rule fixes, output isolation. | `make -n`; isolated build if toolchain available. | moderate |
| `codex/fix-tooling-safety` | Make historical scripts harder to misuse. | Add dry-run/output flags to `src/build.pl`; docs for `deploy.bat`. | Perl syntax tests; dry-run test. | moderate |
| `codex/fix-path-handling` | Reduce launch/path confusion. | Program `zlib.dll` copy-test plan, exact launch matrix docs. | disposable-copy launches. | moderate |
| `codex/fix-gdi-error-reporting` | Improve graphics failure diagnostics. | Source-level checks around image/DIB/config loading. | build proof plus manual launch tests. | risky |
| `codex/investigate-damage-prevention-handlers` | Find remaining freeze candidates. | Inventory `GAA_DAMAGE_PREVENTION` handlers. | source audit plus gameplay repros. | risky |
| `codex/add-static-analysis-ci` | Add optional analyzers once build scope is stable. | cppcheck/semgrep configs. | analyzer reports without runtime changes. | safe |

## Blocked Or Not Done

| Item | Reason |
| --- | --- |
| Full rebuild | Toolchain missing and source provenance unresolved. |
| Runtime/gameplay verification | Out of scope for this static pass. |
| Binary fixes | Explicitly avoided; current patches are protected. |
| Native Windows proof | Needs a Windows environment. |
| Semantic static analysis | Optional tools are not installed. |
