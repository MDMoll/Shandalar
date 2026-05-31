# Shandalar

This repository is a fan-maintained, abandonware-era Shandalar / MicroProse
Magic: The Gathering 1997 checkout. It includes playable Windows binaries,
runtime assets, card art, decks, source fragments, patch scripts, Manalink
materials, updater tools, and historical files.

The safest first step is to treat the repository as a bundled game directory,
not as a clean source tree. Many large files and old-looking folders are art,
deck, sound, card database, or runtime assets.

Distribution status is not verified. The checkout includes third-party game
binaries, art, card data, decks, and trademark/rightsholder notices, and no
repository-level license file was found. See
[docs/distribution.md](docs/distribution.md) before publishing broadly.

Evidence language in these docs is intentional: "verified" means a command was
run on this checkout, "inferred" means the conclusion comes from local strings,
imports, scripts, or filenames, and "needs testing" means it was not proven.

## Quick Start

| User | Start here | Command |
| --- | --- | --- |
| Windows | Open the full checkout and launch directly. | `Shandalar.exe`; use `Program\Magic.exe` for Manalink testing |
| CrossOver on macOS | Use the patched `Shandalar.exe`; the repo copy passed the start-color crash point in `Shandalar-Win8-Test`. | See [docs/crossover-macos.md](docs/crossover-macos.md) |
| Wine | Use the folder that contains the executable and nearby DLLs/assets. | `wine Shandalar.exe` from repo root, or `cd Program` then `wine Magic.exe` |
| Developer/agent | Read the guardrails first. | [AGENTS.md](AGENTS.md) |

The repo root and `Program/` both contain runtime-looking files. Current
CrossOver evidence points at the root copy for Shandalar in bottle `MTG`:
`C:\Shandalar\Shandalar.exe` opens root `C:\Shandalar\Magic.exe`, while direct
`C:\Shandalar\Program\Shandalar.exe` currently fails in that bottle because
`Program\zlib.dll` is missing. The Manalink launcher still enters `Program/`
for `Magic.exe`.

## Launch Targets

| Executable | Purpose | Status from this pass |
| --- | --- | --- |
| `Shandalar.exe` | Shandalar adventure shell and duel path. | Same SHA-256 as `Program/Shandalar.exe`; patched for the start-color `CreateDIBSection` crash, default-name bypass/fallback, and same-arrow adventure-map stop behavior. |
| `Magic.exe` | Duel executable opened by root `Shandalar.exe`. | Different SHA-256 from `Program/Magic.exe`; current CrossOver root launch opens this copy. Patched at `0x43c303`/`0x459bc8` so clicking an already-declared attacker before Done clears it from attack selection. Root `ManalinkEh.dll` is patched for the Samite/Femeref/Kithkin damage-prevention activation freeze. |
| `Program/Shandalar.exe` | Alternate Shandalar copy. | Same SHA-256 as root `Shandalar.exe`; direct `MTG` launch currently fails because `Program/zlib.dll` is absent. |
| `Program/Magic.exe` | Manalink / launcher duel executable. Treat as first-class, not optional. | Present, PE32 i386 GUI executable; launcher scripts enter `Program/` before running it. Has the same declared-attacker undo patch as root `Magic.exe`; `Program/ManalinkEh.dll` has the same Samite/Femeref/Kithkin handler patch at its own offset. |
| `FaceMaker.exe` / `Program/FaceMaker.exe` | Character portrait/name helper launched during new-game setup. | Both active copies are based on the no-resolution/Korath helper, then patched at `0x5f40` so their `CreateDIBSection` wrapper also passes `hSection = NULL`. The reference `*-nores.exe` copies remain unpatched. |
| `Manalink_Launcher.cmd` | Batch launcher/mod helper that enters `Program/` and starts `Magic.exe`. | Inspected, Windows-only batch file. |
| `Shandalar help.bat` | Existing command-line wrapper: `shandalar --e 0442 --p 0442`. | Inspected, semantics still need testing. |

## Launching Shandalar.exe

For the current copied CrossOver install and a normal full-checkout Windows
launch, start with the root executable:

```bat
cd \path\to\Shandalar
Shandalar.exe
```

Use `Program\Shandalar.exe` only when you are deliberately testing the
alternate layout and have verified its adjacent DLLs.

For command-line modes, see [docs/command-line.md](docs/command-line.md).

## Launching Magic.exe

Test `Magic.exe` separately from Shandalar:

```bat
cd \path\to\Shandalar\Program
Magic.exe
```

Also test root `Magic.exe` separately when investigating the Shandalar root
path, because root and `Program/` copies differ by hash. See
[docs/magic-exe.md](docs/magic-exe.md).

## Repository Map

| Path | Practical meaning |
| --- | --- |
| Root runtime files | Current copied CrossOver `MTG` launch surface for `Shandalar.exe`; includes root DLLs, assets, card data, and root `Magic.exe`. |
| `Program/` | Manalink launcher runtime bundle and alternate Shandalar layout. Contains `Shandalar.exe`, `Magic.exe`, DLLs, assets, decks, sounds, and a nested source/updater snapshot. |
| `src/` | C/C++/ASM source and patch tooling for Manalink/CardArt/DrawCard/Deck DLL work. It does not currently prove that the shipped game can be rebuilt end to end. |
| `CardArtManalink/`, `CardArtNew/`, `Cardart/`, `Dbart/`, `Duelart/`, `Exp1art/`, `Shellart/`, `Statwin/` | Art and proprietary game resource stores. Do not delete casually. |
| `decks*`, `Decks*`, `Playdeck/`, `Mods/PlayDeck/` | Player/opponent deck collections and mod deck packs. |
| `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce`, extensionless `MAGIC5`, `Savedescs`, `Screennames/` | Tracked save/local player state and one derived save/deck export. Useful for testing, but review before public release. |
| `Mods/` | Mod archives and mod staging folders used by the launcher. |
| `Manalink3/` | A packaged Manalink 3 style distribution snapshot with its own `Program/`, `Mods/`, and docs. |
| `magic_updater/` | Perl/CSV card data updater tooling. |
| `PlayDeckAnalyser/` | Separate deck analysis utility and configuration. |
| `docs/generated/` | Preserved command output and evidence snapshots for investigations; start with the concise docs first. |
| `local/` | Local CrossOver/Wine helper scripts used for smoke testing, not runtime assets. |
| `tools/` | Repo-maintenance helpers, including automated share-readiness checks. |
| `archive/` | Preserved generated/local/debug/historical files moved out of the root during the limited reorganization pass. Not a trash folder. |

## Verified on this machine

Local checkout path: `/Users/mdmoll/Shandalar/Shandalar`

| Area | Summary |
| --- | --- |
| Runtime inspection | Launch targets are PE32 Windows binaries; import tables and runtime dependencies were inspected. |
| Patched binaries | Active `Shandalar.exe`, `FaceMaker.exe`, `Magic.exe`, and `ManalinkEh.dll` hashes and representative patch bytes match the docs. |
| CrossOver evidence | Root `C:\Shandalar\Shandalar.exe` reached the main menu and patched smoke paths passed the original crash point, but full gameplay still needs visible manual testing. |
| Repo hygiene | `tools/verify-share-readiness.sh` passes clean-tree, binary attribute, patched hash, patch-byte, core-doc, and local-link checks. |

See [docs/verified-on-this-machine.md](docs/verified-on-this-machine.md) for
the detailed command/result table.

## Known Issues

| Issue | What to do |
| --- | --- |
| `Shandalar.exe --help` output was not captured locally. | See [docs/command-line.md](docs/command-line.md); test in a visible Windows/CrossOver session. |
| `Program/` CrossOver launch attempts exited code 53 in the existing `MTG` bottle. | Do not use direct `C:\Shandalar\Program\Shandalar.exe` as the primary `MTG` retest until `Program\zlib.dll` and related DLL layout are resolved. |
| Duel prompts stop accepting `Done`, `Trigger`, or `Decline` in CrossOver. | Root and `Program/` `ManalinkEh.dll` are patched for the Samite/Femeref/Kithkin damage-prevention handler that can be reached by Femeref Healer during combat. The local `MTG` bottle copies were updated with backups; visible gameplay retesting still needs to confirm the Femeref Healer blocker scenario. |
| Declared attacker mistakes are hard to undo. | Root and `Program/` `Magic.exe` now have a conservative attacker-selection undo patch, and the local `MTG` bottle copies were updated with backups. Before pressing Done, clicking an already-declared attacker should clear `STATE_ATTACKING`; unusual attack costs, attack triggers, and banding still need visible testing. See [docs/bugs/declared-attacker-undo.md](docs/bugs/declared-attacker-undo.md). |
| Start-color `WM_CREATE CreateDIBSection` assertion, name-entry glitch, and map movement stop | FaceMaker/no-resolution and bottle-setting fixes did not solve the original issue. Root and `Program/` `Shandalar.exe` include a narrow `CreateDIBSection` compatibility patch, a default-name seed plus name-editor bypass/fallback, and a same-arrow movement-stop patch; the active FaceMaker copies have the same `hSection = NULL` patch at their own DIB wrapper. Direct patched FaceMaker startup is verified; full Shandalar-spawned character creation and movement control still need a visible manual retest. See [docs/troubleshooting.md](docs/troubleshooting.md), [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md), and [docs/adventure-map-movement.md](docs/adventure-map-movement.md). |
| Runtime advice is mixed in old docs. | Use [docs/runtime-dependencies.md](docs/runtime-dependencies.md), which separates import evidence from historical notes. |
| Antivirus/security state is unknown. | No malware scanner result is recorded for this branch. See [docs/security-scan.md](docs/security-scan.md) before making safety claims. |
| There are many duplicates and historical files. | Use [docs/cleanup-audit.md](docs/cleanup-audit.md). Nothing should be deleted without a manual test plan. |
| Obvious non-runtime historical/local clutter was moved under `archive/`. | Use [archive/README.md](archive/README.md) and [docs/reorganization.md](docs/reorganization.md). Runtime root and `Program/` files were not archived. |

## Needs testing

| Area | Next proof needed |
| --- | --- |
| Native Windows launch | Run root `Shandalar.exe`, root `Magic.exe`, and `Program/Magic.exe` from their own folders and record visible behavior. |
| CrossOver gameplay retest | Run the patched `Shandalar.exe` visibly, then verify character creation reaches the map with default `Player`, same-arrow map stop, save/load, a duel, and all five starting colors. |
| Declared attacker undo | In a duel, declare one or more ordinary attackers, click a declared attacker again before Done, and confirm it leaves the attack selection without becoming tapped or marked as having attacked. |
| Command-line help | Capture `Shandalar.exe --help` text, dialog, or log output. |
| Root vs `Program/` `Magic.exe` | Test both exact paths because their hashes differ. |
| Security scan | Run a named scanner and record hashes/results; use `tools/print-security-scan-baseline.sh` to prepare the evidence table. |

## More Docs

| Doc | Use it for |
| --- | --- |
| [docs/README.md](docs/README.md) | Documentation index grouped by task. |
| [docs/branch-summary.md](docs/branch-summary.md) | Handoff summary for this cleanup/runtime branch. |
| [docs/running.md](docs/running.md) | Windows, CrossOver, Wine launch commands and test matrix. |
| [docs/share-readiness.md](docs/share-readiness.md) | What is ready to share and what still needs proof. |
| [docs/completion-audit.md](docs/completion-audit.md) | Requirement-by-requirement status for the cleanup/share goal. |
| [docs/git-handoff.md](docs/git-handoff.md) | Exact branch, remote, pre-push checks, and push-auth status. |
| [docs/release-scope.md](docs/release-scope.md) | Current branch sharing scope and public-release boundaries. |
| [docs/manual-gameplay-verification.md](docs/manual-gameplay-verification.md) | Manual test plan required before claiming the game works end to end. |
| [docs/crossover-macos.md](docs/crossover-macos.md) | Practical CrossOver bottle setup and troubleshooting. |
| [docs/magic-exe.md](docs/magic-exe.md) | Dedicated `Magic.exe` notes, imports, hypotheses, and tests. |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Practical troubleshooting entries, including the start-color DIB assertion. |
| [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md) | Focused investigation of the `WM_CREATE CreateDIBSection` assertion after choosing a start color. |
| [docs/bugs/duel-freeze-damage-prevention.md](docs/bugs/duel-freeze-damage-prevention.md) | Focused investigation of the Femeref/Samite/Kithkin damage-prevention activation freeze. |
| [docs/bugs/declared-attacker-undo.md](docs/bugs/declared-attacker-undo.md) | Focused notes for the `Magic.exe` attacker-selection undo patch. |
| [docs/magic-vs-shandalar-runtime.md](docs/magic-vs-shandalar-runtime.md) | Runtime comparison notes for `Magic.exe` and `Shandalar.exe`. |
| [docs/runtime-manifest.md](docs/runtime-manifest.md) | SHA-256 manifest for active patched files and protected references. |
| [docs/runtime-dependencies.md](docs/runtime-dependencies.md) | PE inspection and runtime dependency matrix. |
| [docs/security-scan.md](docs/security-scan.md) | Antivirus/security scan guidance and reporting template. |
| [docs/distribution.md](docs/distribution.md) | Distribution/licensing caution for sharing this bundled game tree. |
| [docs/verified-on-this-machine.md](docs/verified-on-this-machine.md) | Detailed local command/result evidence. |
| [docs/command-line.md](docs/command-line.md) | `--help`, `--e`, `--p`, and command-line evidence. |
| [docs/save-state.md](docs/save-state.md) | Tracked save slots, screen names, and cleanup plan. |
| [docs/cleanup-move-plan.md](docs/cleanup-move-plan.md) | Exact pending cleanup moves that need explicit approval or launch-copy testing. |
| [docs/cleanup-launch-copy-test.md](docs/cleanup-launch-copy-test.md) | Disposable-copy test plan required before moving risky cleanup candidates. |
| [docs/architecture.md](docs/architecture.md) | High-level repo organization. |
| [docs/building.md](docs/building.md) | What appears buildable and current blockers. |
| [docs/file-inventory.md](docs/file-inventory.md) | Counts, large files, duplicate observations, and runtime-critical file types. |
| [docs/duplicate-audit.md](docs/duplicate-audit.md) | Full non-git SHA-256 duplicate summary and cleanup confidence. |
| [docs/cleanup-audit.md](docs/cleanup-audit.md) | Cleanup candidates with confidence and evidence. |
| [docs/stale-references.md](docs/stale-references.md) | Old URLs, hard-coded paths, and historical references. |
| [docs/reorganization.md](docs/reorganization.md) | Exact limited-reorg moves, rationale, commands, and verification. |
| [docs/gaps.md](docs/gaps.md) | Remaining launch, command-line, duplicate, and build gaps. |
| [docs/adventure-map-movement.md](docs/adventure-map-movement.md) | Static notes and verification commands for the same-arrow adventure-map stop patch. |
| [docs/generated/README.md](docs/generated/README.md) | Map for generated evidence snapshots. |
| [local/README.md](local/README.md) | Local-only CrossOver/Wine helper scripts and scope notes. |
| [tools/README.md](tools/README.md) | Repo-maintenance helper scripts and verifier usage. |

## Automated Checks

Run the non-gameplay share-readiness checks with:

```sh
tools/verify-share-readiness.sh
```

This verifies the clean tree, ignored local clutter, generated report/handoff
ignore rules,
expected tracked ignored file, Git binary attributes, protected cleanup false
positives, patched runtime hashes, runtime-manifest hashes, representative patch
bytes, tracked save/local-state inventory, security-scan target inventory, core
docs, maintained-text ASCII, docs index coverage, and local Markdown links. It
does not replace the manual gameplay checklist.

## Evidence Helpers

These helpers do not prove gameplay or security by themselves, but they reduce
handoff mistakes when a human is ready to test:

```sh
tools/print-manual-gameplay-baseline.sh
tools/print-security-scan-baseline.sh
tools/verify-handoff-readiness.sh
tools/create-cleanup-test-copy.sh --dry-run
tools/create-git-handoff-bundle.sh --dry-run
```
