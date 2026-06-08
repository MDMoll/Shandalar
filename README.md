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
Runtime testing is intentionally bounded: do not let Wine/CrossOver GUI
automation become the main work loop. See
[docs/runtime-testing-policy.md](docs/runtime-testing-policy.md).

## Quick Start

| User | Start here | Command |
| --- | --- | --- |
| Windows | Open the full checkout and launch directly. | `Shandalar.exe`; use `Program\Magic.exe` for Manalink testing |
| CrossOver on macOS | Use the patched copied install in bottle `MTG`: root `C:\Shandalar\Shandalar.exe`, app-default `win7`, desktop `Shandalar1440=1440x1080`, and `ShowCoinFlips=0`. | See [docs/crossover-macos.md](docs/crossover-macos.md) |
| Wine | Use the folder that contains the executable and nearby DLLs/assets. | `wine Shandalar.exe` from repo root, or `cd Program` then `wine Magic.exe` |
| Developer/agent | Read the guardrails first. | [AGENTS.md](AGENTS.md) |

The repo root and `Program/` both contain runtime-looking files. Current
CrossOver evidence points at the root copy for Shandalar in bottle `MTG`:
`C:\Shandalar\Shandalar.exe` opens root `C:\Shandalar\Magic.exe`, while direct
`C:\Shandalar\Program\Shandalar.exe` previously failed in that copied bottle
when `Program\zlib.dll` was absent. This checkout now includes
`Program\zlib.dll` as a byte-for-byte copy of root `zlib.dll`. A later visible
direct-Program launch then reached `drawcardlib.dll` and reported missing
`C:\Shandalar\Program\CARDART\ManaSymbols.pic`; this checkout and the local
`MTG` copied install now include the Program CardArt files observed so far
listed in [docs/runtime-manifest.md](docs/runtime-manifest.md), including the
later `Modern/Triggering.png`, `Planeswalker/Loyalty*.png`, and
`Modern/CardOv_Nyx.png` findings. Follow-up logged Program-path runs also
exposed missing `C:\Shandalar\Program\Manalink.ini`, `DuelArt\Modern.dat`,
`DuelArt\Planeswalker.dat`, and six hardcoded Program `TT*.ttf` drawcard fonts;
those are now present in the checkout and local copied install. A later visible
fatal while building the Hornet token showed the older Program card data was
too short for an expected card id; after copying `Program/Cards.dat` and
`Program/Rarity.dat`, a follow-up dialog exposed the still-older
`Program/DBInfo.dat`. All three Program card metadata files now match the newer
root trio in both places. A later visible recurrence of the Hornet fatal traced
to the older `Program/Shandalar.dll` helper generation, not to the already-synced
Program data trio; `Program/Shandalar.dll`, `Program/CardArtLib.dll`,
`Program/Deckdll.dll`, and `Program/Drawcardlib.dll` now match their root
counterparts, and `Program/libgcc_s_dw2-1.dll` matches root too for the newer
drawcard helper. A bounded post-copy log,
`/tmp/shandalar-mtg-program-post-dbinfo-visible-retest-cx.log`, opened
`C:\Shandalar\Program\Cards.dat`, `DBInfo.dat`, `Rarity.dat`,
`shandalar.dll`, and `Shandalar.ini`, and a targeted scan did not show the
generic card-data fatal, Hornet fatal, missing-asset dialog, page fault, or
unhandled exception strings. The latest bounded Program-helper retest,
`/tmp/shandalar-mtg-program-dll-sync-libgcc-retest-cx.log`, loaded
`DrawCardLib.dll`, `libgcc_s_dw2-1.dll`, `DECKDLL.dll`, `shandalar.dll`, and
the Program card-data files without those old fatal strings, then stayed alive
until the alarm. This is still log evidence only, so the exact copied-bottle
Program path needs a visible retest before becoming the primary CrossOver
target. The Manalink launcher still enters
`Program/` for `Magic.exe`. The current `MTG` retest setting is
app-default `win7` with a
Wine virtual desktop named `Shandalar1440` at `1440x1080`; the separate
`Shandalar-Win8-Test` bottle is comparison evidence, not the primary path.

## Launch Targets

| Executable | Purpose | Status from this pass |
| --- | --- | --- |
| `Shandalar.exe` | Shandalar adventure shell and duel path. | Same SHA-256 as `Program/Shandalar.exe`; patched for the start-color `CreateDIBSection` crash, default-name bypass/fallback, same-arrow adventure-map stop behavior, and CrossOver/Wine callback stability around the legacy MagSnd/WinMM/MCI paths. |
| `Magic.exe` | Duel executable opened by root `Shandalar.exe`. | Different SHA-256 from `Program/Magic.exe`; current CrossOver root launch opens this copy. Patched at `0x43c303`/`0x459bc8` so clicking an already-declared attacker before Done clears it from attack selection, and at file offset `0x5db1f` so missing `ShowCoinFlips` registry data defaults coin-flip animations off. Root `ManalinkEh.dll` is patched for Samite-family and generic activated damage-prevention prompt freezes, AI decision-time clamping, and AI raw-mana speculation snapshot restore safety. |
| `Program/Shandalar.exe` | Alternate Shandalar copy. | Same SHA-256 as root `Shandalar.exe`; `Program/zlib.dll`, Program helper DLLs, `Program/libgcc_s_dw2-1.dll`, adjacent Program config/font/card-data files, and Program `CardArt` drawcardlib assets observed so far are now present in this checkout and the local `MTG` copied install. Direct `MTG` copied-bottle launch still needs visible retesting before using it as the primary Shandalar path. |
| `Program/Magic.exe` | Manalink / launcher duel executable. Treat as first-class, not optional. | Present, PE32 i386 GUI executable; launcher scripts enter `Program/` before running it. Has the same declared-attacker undo and `ShowCoinFlips` default-off patches as root `Magic.exe`; `Program/ManalinkEh.dll` has the same damage-prevention, AI clamp, and AI raw-mana snapshot patches at its own offsets. |
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
alternate layout and have verified its adjacent DLLs and top-level card-art
assets.

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
| `Mods/` | Canonical mod archives and mod staging folders used by the active root launcher. |
| `Manalink3/` | Historical/unsupported Manalink 3 package snapshot. Duplicate archives under `Manalink3/Mods/` were removed after top-level `Mods/` was selected as canonical. |
| `magic_updater/` | Perl/CSV card data updater tooling. |
| `PlayDeckAnalyser/` | Separate deck analysis utility and configuration. |
| `DeckInjector.jar`, `src/gui/` | Java Swing Deck Injector support tool and source; build with JDK 21. It is not part of the Shandalar/Magic runtime path. |
| `docs/generated/` | Preserved command output and evidence snapshots for investigations; start with the concise docs first. |
| `local/` | Local CrossOver/Wine helper scripts used for smoke testing, not runtime assets. |
| `tools/` | Repo-maintenance helpers, including current share-status reports, automated readiness checks, and handoff artifacts. |
| `archive/` | Preserved generated/local/debug/historical files moved out of the root during the limited reorganization pass. Not a trash folder. |

## Verified on this machine

Local checkout path: `/Users/mdmoll/Shandalar/Shandalar`

| Area | Summary |
| --- | --- |
| Runtime inspection | Launch targets are PE32 Windows binaries; import tables and runtime dependencies were inspected. |
| Patched binaries | Active `Shandalar.exe`, `FaceMaker.exe`, `Magic.exe`, and `ManalinkEh.dll` hashes and representative patch bytes match the docs. |
| CrossOver evidence | Root `C:\Shandalar\Shandalar.exe` reached the main menu and the default/first start-color path reached the adventure map in `MTG`; the current bottle state validates as app-default `win7` with `Shandalar1440=1440x1080` and `ShowCoinFlips=0`, but full gameplay still needs visible manual testing. |
| Repo hygiene | `tools/verify-share-readiness.sh` checks tracked fresh-install payloads, patched hashes/bytes, install-tree verification, binary attributes, core docs, and local links. |

See [docs/verified-on-this-machine.md](docs/verified-on-this-machine.md) for
the detailed command/result table.

## Known Issues

| Issue | What to do |
| --- | --- |
| `Shandalar.exe --help` output was not captured locally. | See [docs/command-line.md](docs/command-line.md); test in a visible Windows/CrossOver session. |
| `Program/` CrossOver launch attempts exposed missing adjacent runtime files in the existing `MTG` bottle. | The local `MTG` copied install now has matching root and Program zlib DLLs, matching Program helper DLLs (`Shandalar.dll`, `CardArtLib.dll`, `Deckdll.dll`, and `Drawcardlib.dll`), matching `Program/libgcc_s_dw2-1.dll`, matching `Program/Manalink.ini`, matching `Program/Cards.dat`, `Program/DBInfo.dat`, and `Program/Rarity.dat`, Program `DuelArt` configs through `Planeswalker.dat`, the six hardcoded Program `TT*.ttf` drawcard fonts, plus matching Program CardArt files for `ManaSymbols.pic`, `Expansion_Symbols.pic`, `Watermarks.pic`, `CardCounters.png`, `Modern/Triggering.png`, `Modern/CardOv_Nyx.png`, and the four `Planeswalker/Loyalty*.png` images. A bounded post-helper-sync log loaded the Program helper DLLs and card-data trio without the earlier fatal strings, but keep root `C:\Shandalar\Shandalar.exe` as the primary `MTG` retest until direct `C:\Shandalar\Program\Shandalar.exe` has a visible exact-path retest. |
| Duel freezes before the starting coin flip. | Root and `Program/` `Magic.exe` now default missing `ShowCoinFlips` registry data to `0`, and the local `MTG` bottle's explicit `DuelOptions` registry value is also set to `ShowCoinFlips=0`. This targets the AVI/dialog animation path before the duel begins, but visible duel-start retesting is still pending. See [docs/bugs/duel-start-coinflip-animation.md](docs/bugs/duel-start-coinflip-animation.md). |
| Duel prompts stop accepting `Done`, `Trigger`, or `Decline` in CrossOver. | Root and `Program/` `ManalinkEh.dll` are patched for the Samite/Femeref/Kithkin damage-prevention handler, the generic activated `GAA_DAMAGE_PREVENTION*` helper, `AiDecisionTime` values that are missing, invalid, or above 270, and an AI raw-mana speculation snapshot restore bug. Renderer helpers have also been rebuilt so Drawcardlib and CardArtLib use safer GDI+ startup/background-hook handling, external-codec suppression, and legacy-safe PE flags; see [docs/bugs/gdiplus-renderer-risk-audit.md](docs/bugs/gdiplus-renderer-risk-audit.md). The local `MTG` bottle copies were updated with the same patches and backups; visible gameplay retesting still needs to confirm Spell Chain/card-rendering stability, ordinary spell casting, and broader opponent-turn behavior. |
| Declared attacker mistakes are hard to undo. | Root and `Program/` `Magic.exe` now have a conservative attacker-selection undo patch, and the local `MTG` bottle copies were updated with backups. Before pressing Done, clicking an already-declared attacker should clear `STATE_ATTACKING`; unusual attack costs, attack triggers, and banding still need visible testing. See [docs/bugs/declared-attacker-undo.md](docs/bugs/declared-attacker-undo.md). |
| Start-color `WM_CREATE CreateDIBSection` assertion, name-entry glitch, and map movement stop | FaceMaker/no-resolution and bottle-setting fixes did not solve the original issue. Root and `Program/` `Shandalar.exe` include a narrow `CreateDIBSection` compatibility patch, a default-name seed plus name-editor bypass/fallback, and a same-arrow movement-stop patch; the active FaceMaker copies have the same `hSection = NULL` patch at their own DIB wrapper. Direct patched FaceMaker startup is verified, and one visible default/first start-color path reached the adventure map. Remaining colors, save/load, and movement control still need visible retesting. See [docs/troubleshooting.md](docs/troubleshooting.md), [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md), and [docs/adventure-map-movement.md](docs/adventure-map-movement.md). |
| Runtime advice is mixed in old docs. | Use [docs/runtime-dependencies.md](docs/runtime-dependencies.md), which separates import evidence from historical notes. |
| Security scan scope is narrow. | ClamAV reported no infected files for the 241 tracked security-scan targets, but that is not a broad safety or redistribution claim. See [docs/security-scan.md](docs/security-scan.md). |
| There are many duplicates and historical files. | Use [docs/cleanup-audit.md](docs/cleanup-audit.md). Nothing should be deleted without a manual test plan. |
| Obvious non-runtime historical/local clutter was moved under `archive/`. | Use [archive/README.md](archive/README.md) and [docs/reorganization.md](docs/reorganization.md). Runtime root and `Program/` files were not archived. |

## Needs testing

| Area | Next proof needed |
| --- | --- |
| Native Windows launch | Run root `Shandalar.exe`, root `Magic.exe`, and `Program/Magic.exe` from their own folders and record visible behavior. |
| CrossOver gameplay retest | Run the patched `Shandalar.exe` visibly, then verify the remaining starting colors, same-arrow map stop, save/load, a duel, and the regression scenarios. |
| Declared attacker undo | In a duel, declare one or more ordinary attackers, click a declared attacker again before Done, and confirm it leaves the attack selection without becoming tapped or marked as having attacked. |
| Command-line help | Capture `Shandalar.exe --help` text, dialog, or log output. |
| Root vs `Program/` `Magic.exe` | Test both exact paths because their hashes differ. |
| Additional scanner review | ClamAV evidence is recorded; use `tools/print-security-scan-baseline.sh` if another trusted scanner is run later. |

## More Docs

| Doc | Use it for |
| --- | --- |
| [docs/README.md](docs/README.md) | Documentation index grouped by task. |
| [docs/branch-summary.md](docs/branch-summary.md) | Handoff summary for this cleanup/runtime branch. |
| [docs/running.md](docs/running.md) | Windows, CrossOver, Wine launch commands and test matrix. |
| [docs/share-readiness.md](docs/share-readiness.md) | What is ready to share and what still needs proof. |
| [docs/completion-audit.md](docs/completion-audit.md) | Requirement-by-requirement status for the cleanup/share goal. |
| [docs/git-handoff.md](docs/git-handoff.md) | Exact branch, remote, pre-push checks, and push-auth status. |
| [docs/push-auth.md](docs/push-auth.md) | Owner-side GitHub authentication options for pushing this branch. |
| [docs/release-scope.md](docs/release-scope.md) | Current branch sharing scope and public-release boundaries. |
| [docs/patch-package-plan.md](docs/patch-package-plan.md) | Branch-delta inventory and restoration-test plan for any future patch/docs-only package. |
| [docs/manual-gameplay-verification.md](docs/manual-gameplay-verification.md) | Manual test plan required before claiming the game works end to end. |
| [docs/runtime-testing-policy.md](docs/runtime-testing-policy.md) | Bounded Wine/CrossOver/native Windows runtime-testing rules and stop conditions. |
| [docs/runtime-test-notes.md](docs/runtime-test-notes.md) | Short chronological notes from bounded runtime attempts. |
| [docs/crossover-macos.md](docs/crossover-macos.md) | Practical CrossOver bottle setup and troubleshooting. |
| [docs/magic-exe.md](docs/magic-exe.md) | Dedicated `Magic.exe` notes, imports, hypotheses, and tests. |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Practical troubleshooting entries, including the start-color DIB assertion. |
| [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md) | Focused investigation of the `WM_CREATE CreateDIBSection` assertion after choosing a start color. |
| [docs/bugs/duel-freeze-damage-prevention.md](docs/bugs/duel-freeze-damage-prevention.md) | Focused investigation of the Femeref/Samite/Kithkin damage-prevention activation freeze. |
| [docs/bugs/ai-raw-mana-snapshot.md](docs/bugs/ai-raw-mana-snapshot.md) | Focused `ai.c` finding and runtime patch for the AI speculation raw-mana restore bug. |
| [docs/bugs/opponent-turn-ai-decision-time.md](docs/bugs/opponent-turn-ai-decision-time.md) | Focused notes for the AI speculation timer and fallback patch. |
| [docs/bugs/duel-start-coinflip-animation.md](docs/bugs/duel-start-coinflip-animation.md) | Focused notes for the duel-start coin-flip animation default and `MTG` registry setting. |
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
| [docs/deck-injector.md](docs/deck-injector.md) | JDK 21 build and verification notes for the Java Deck Injector support tool. |
| [docs/file-inventory.md](docs/file-inventory.md) | Counts, large files, duplicate observations, and runtime-critical file types. |
| [docs/install-roots.md](docs/install-roots.md) | Install-root inventory and supported-layout decisions. |
| [docs/package-layout-cleanup.md](docs/package-layout-cleanup.md) | Package-layout cleanup decision log and exact duplicate archive removals. |
| [docs/duplicate-audit.md](docs/duplicate-audit.md) | Full non-git SHA-256 duplicate summary and cleanup confidence. |
| [docs/duplicate-cleanup-verification.md](docs/duplicate-cleanup-verification.md) | Verified duplicate removal evidence, protected families, archive policy, and quarantine result. |
| [docs/cleanup-audit.md](docs/cleanup-audit.md) | Cleanup candidates with confidence and evidence. |
| [docs/cleanup-removed-files.md](docs/cleanup-removed-files.md) | Exact safe cleanup removals and rollback notes. |
| [docs/repo-cleanup-plan.md](docs/repo-cleanup-plan.md) | Current cleanup decision log and next cleanup batches. |
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
tools/print-share-status.sh
tools/verify-share-readiness.sh
```

This verifies the clean tree, ignored local clutter, generated report/handoff
ignore rules, absence of tracked ignored files, Git binary attributes, protected
cleanup false positives, tracked fresh-install runtime/support files, patched
runtime hashes, runtime-manifest hashes, representative patch bytes, tracked
save/local-state inventory, security-scan target inventory, core docs,
branch-delta inventory shape, current top-level CrossOver guidance,
runtime-testing policy indexing, maintained-text ASCII, docs index coverage,
and local Markdown links. It also runs the generic install-tree verifier against
the repo root. It does not replace the manual gameplay checklist.

To check a fresh copied install tree without relying on the local CrossOver
`MTG` bottle, run:

```sh
tools/verify-install-tree.sh /path/to/Shandalar
```

## Evidence Helpers

These helpers do not prove gameplay or security by themselves, but they reduce
handoff mistakes when a human is ready to test:

```sh
tools/print-manual-gameplay-baseline.sh
tools/check-security-scanner-availability.sh
tools/print-security-scan-baseline.sh
tools/print-share-status.sh
tools/verify-manual-gameplay-results.sh --allow-incomplete --show-missing
tools/record-manual-gameplay-result.sh --test D2 --result "Fail: froze at post-combat Done; screenshot /path/to/screenshot.png"
tools/create-security-scan-results-template.sh --output security-scan-results.tsv
tools/record-security-scan-result.sh --confirmed-real-scan --all-current-targets --replace-row --scanner "Windows Defender" --version "VERSION" --date 2026-05-31 --result "Clean" --notes "MpCmdRun.exe custom scan completed"
tools/verify-final-share-gates.sh
tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts
tools/create-git-handoff-bundle.sh --replace
tools/create-patch-package.sh --replace --verify-apply
tools/verify-install-tree.sh /path/to/Shandalar
tools/create-patch-package.sh --dry-run
tools/list-branch-delta.sh
tools/list-branch-delta.sh --summary
tools/create-cleanup-test-copy.sh --dry-run
tools/create-git-handoff-bundle.sh --dry-run
```

`tools/verify-final-share-gates.sh` is intentionally strict. It should fail
until manual gameplay rows are complete; it also verifies named security-scan
coverage and the pushed branch when the evidence gates are otherwise ready.
