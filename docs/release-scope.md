# Release Scope

This file records the current sharing decision for
`codex/shandalar-crossover-updates`.

## Current Decision

| Scope | Decision | Meaning |
| --- | --- | --- |
| Controlled maintenance branch | Current branch scope | Share only with recipients who already understand the bundled game/runtime nature of this checkout and the same rights context. |
| Public full bundle | Not approved | Do not publish this repository as a public asset/game bundle from the current evidence. |
| Patch/docs-only package | Not prepared | A smaller package may be easier to share, but no file list, patch format, installer, or restoration workflow has been prepared. |
| Rights-verified public bundle | Not proven | No repository-level license was found, and bundled rightsholder/trademark notices are present. |

This is a practical repo-maintenance decision, not legal advice.

## What Is Safe To Claim

| Claim | Status |
| --- | --- |
| The branch is cleaner and better documented for local maintenance. | Supported by [share-readiness.md](share-readiness.md) and [completion-audit.md](completion-audit.md). |
| The branch can be pushed to git once authentication works. | Supported locally by a clean tree and `tools/verify-share-readiness.sh`; remote push is still blocked by local GitHub auth. |
| The branch is a public release. | Do not claim. |
| The binaries are malware-scanned. | Do not claim until [security-scan.md](security-scan.md) has named scanner/version/hash results. |
| Gameplay is fully verified. | Do not claim until [manual-gameplay-verification.md](manual-gameplay-verification.md) has concrete pass/fail results. |

## Current Full-Bundle Contents

| Area | Evidence | Sharing implication |
| --- | --- | --- |
| Runtime binaries and DLLs | [runtime-manifest.md](runtime-manifest.md), [security-scan.md](security-scan.md), and `tools/list-security-scan-targets.sh`. | Needs rights and scan review before public redistribution. |
| Art, sounds, videos, decks, and card data | [file-inventory.md](file-inventory.md), [duplicate-audit.md](duplicate-audit.md), and [distribution.md](distribution.md). | Treat as bundled game/runtime content, not disposable cleanup clutter. |
| Save slots and player/screen-name state | [save-state.md](save-state.md). | Needs manual save/load review before deciding whether it is fixture data or local state. |
| Historical and generated evidence | [reorganization.md](reorganization.md), [archive/README.md](../archive/README.md), and [generated/README.md](generated/README.md). | Preserved for auditability; not proof of public release rights. |

## If Preparing A Different Release

| Candidate release | Required work before calling it ready |
| --- | --- |
| Private maintenance branch | Push from an authenticated environment, keep this release scope visible, and do not make public redistribution or scanner claims. |
| Patch/docs-only package | Decide a patch format, list exact source files and binary-diff artifacts, test restoration against a clean local install, and document what original assets the recipient must provide. |
| Rights-verified full bundle | Verify rights for binaries, art, card data, deck packs, sounds, videos, archives, and fan-maintained files; run named malware scans; complete manual gameplay verification. |

Until one of the future release paths is prepared and verified, treat this
branch as a controlled maintenance branch only.
