# Shandalar

This repository is a fan-maintained, abandonware-era Shandalar / MicroProse
Magic: The Gathering 1997 checkout. It includes playable Windows binaries,
runtime assets, card art, decks, source fragments, patch scripts, Manalink
materials, updater tools, and historical files.

The safest first step is to treat the repository as a bundled game directory,
not as a clean source tree. Many large files and old-looking folders are art,
deck, sound, card database, or runtime assets.

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
| `archive/` | Preserved generated/local/debug/historical files moved out of the root during the limited reorganization pass. Not a trash folder. |

## Verified on this machine

Local checkout path: `/Users/mdmoll/Shandalar/Shandalar`

| Check | Result |
| --- | --- |
| `file Program/Shandalar.exe Program/Magic.exe` | Both are `PE32 executable (GUI) Intel 80386, for MS Windows`. |
| `objdump -p Program/Shandalar.exe` | Imports Win32 DLLs plus `DrawCardLib.dll`, `DECKDLL.dll`, `CdTools.dll`, `CardArtLib.dll`, and `MSVCRT.dll`. |
| `objdump -p Program/Magic.exe` | Imports Win32 DLLs plus `deckdll.dll`, `drawcardlib.dll`, `manalinkeh.dll`, and `manalinkex.dll`. |
| `shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe` | Active `FaceMaker.exe` copies hash to `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`; reference no-resolution/Korath copies hash to `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`. |
| `xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe Program/FaceMaker.exe` | Both active FaceMaker dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing their `CreateDIBSection` wrapper to receive `hSection = NULL`. |
| `cmp -s /private/tmp/FaceMaker-Korath-thread.exe FaceMaker-nores.exe` plus `find /Users/mdmoll/Shandalar -iname '*facemaker*korath*'` | The known downloaded thread helper matches `FaceMaker-nores.exe`; no repo file named `Facemaker-Korath.exe` was visible under `/Users/mdmoll/Shandalar` during this pass. |
| `diff -qr Program/FaceArt Manalink3/Program/FaceArt` | No differences; FaceMaker art support is present in `Program/`. |
| `shasum -a 256 Shandalar.exe Program/Shandalar.exe` | Root and `Program/` `Shandalar.exe` match: `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`. |
| `xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe` and `Program/Shandalar.exe` | Both dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing the Shandalar `CreateDIBSection` wrapper to receive `hSection = NULL`. |
| `xxd -g1 -l 40 -s $((0xa1a42)) Shandalar.exe` and `Program/Shandalar.exe` | Both dumps seed `mPlayer` at `0x591228`, avoiding a Wine-fragile pixel-copy read from the name picker surface before the existing code strips the leading gender byte. |
| `xxd -g1 -l 32 -s $((0xa1acd)) Shandalar.exe` and `Program/Shandalar.exe` | Both dumps begin `31 c0 89 85 a8 fe ff ff`, bypassing the fragile name editor and continuing with the seeded default name. |
| `xxd -g1 -l 64 -s $((0x64570)) Shandalar.exe` and `Program/Shandalar.exe` | Both dumps contain a fallback guard that writes `Player` if the accepted name buffer at `0x591228` is empty before copying it to `0x7a0770`. |
| `shasum -a 256 Magic.exe Program/Magic.exe` | Root and `Program/` `Magic.exe` differ after the declared-attacker undo patch: `5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1` and `0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa`. |
| Separate `xxd` checks at `0x43c303` and `0x459bc8` for `Magic.exe` and `Program/Magic.exe` | Both `Magic.exe` copies have the declared-attacker undo hook. The hook begins `e9 c0 d8 01 00 90 90 90 90 90 90 90 90`; the cave begins `f7 46 08 04 00 00 00 0f 85 12 00 00 00`. |
| `shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll` | Root patched hash is `6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb`; `Program/` patched hash is `7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517`. |
| `xxd -g1 -l 32 -s $((0x3bb035)) ManalinkEh.dll` and `xxd -g1 -l 32 -s $((0x381a25)) Program/ManalinkEh.dll` | Both patched dumps begin `f6 05 90 f1 4e 00 04 0f 84 ae 00 00 00 e9 01 00`, gating the Samite/Femeref/Kithkin healer handler on the engine's damage-prevention window. |
| CrossOver `MTG` shortcut inspection | The visible shortcut targets `C:\Shandalar\Shandalar.exe`, not `C:\Shandalar\Program\Shandalar.exe`. |
| Direct `MTG` launch of `C:\Shandalar\Program\Shandalar.exe` with logging | Fails before gameplay because `Program\zlib.dll` is missing. |
| `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini` | Both repo configs are set to `Window = 2` for the next start-color test. |
| `rg -n "^Window\\s*=" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.ini" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.ini"` | The copied install inside bottle `MTG` is also set to `Window = 2`. |
| `rg -n "PagingFiles" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/system.reg"` | The local `MTG` bottle was updated from `C:\pagefile.sys 27 77` to `C:\pagefile.sys 512 1024`. |
| `sed -n '795,825p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"` | Bottle `MTG` currently sets `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` to app-default `Version=win7` and desktop `Shandalar1440`; `Shandalar1440` is `1440x1080`. |
| Timed launch of `C:\Shandalar\Shandalar.exe` without explicit `--desktop` | Stayed alive until the alarm; log showed `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. |
| Direct launch of patched `C:\Shandalar\FaceMaker.exe` | Rendered the FaceMaker UI in bottle `MTG`; `/tmp/facemaker-direct-after-patch-cx.log` showed no unhandled exception, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| Visual launch of `C:\Shandalar\Shandalar.exe` | Reached the `Magic: Shandalar` main menu. Later AppleScript/Wine SendKeys/Swift click attempts still could not drive the menu from this machine, so full Shandalar-spawned character creation remains a manual visual test. |
| Created `Shandalar-Win8-Test` CrossOver bottle | Real 32-bit `win8` template, `WineArch=win32`, Windows registry reports `CurrentVersion=6.2`, `CurrentBuild=9200`, `ProductName=Microsoft Windows 8`. |
| Timed launch of `Y:\Shandalar\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Stayed alive until killed after the timed smoke; log showed `wined3d` and many successful `NtGdiCreateDIBSection` calls. |
| Copied checkout into `Shandalar-Win8-Test` | Bottle-local install exists at `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar`, exposed to Wine as `C:\Shandalar`. |
| Configured `Shandalar-Win8-Test` taller virtual desktop | `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe` use desktop `ShandalarTall`, size `1024x800`. This is based on logged 1024x800-ish DIB creation, not yet a proven start-color fix. |
| Timed launch of `C:\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Stayed alive until cleanup after the timed smoke; log showed the C-drive install path and successful DIB creation. |
| Binary patches applied to root and `Program/` `Shandalar.exe` | File offset `0x1785b0` passes `hSection = NULL` to `CreateDIBSection`; `0xa1a42` seeds `mPlayer`; `0xa1acd` bypasses the fragile name editor; `0xa1af2` jumps to a fallback guard at `0x465170`/file offset `0x64570`; movement hooks at `0x44398c`, `0x444a2b`, and `0x444aa7` use code cave `0x46502d` and flag `0x583a2c` for same-arrow stop. Both files hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`. |
| Local CrossOver bottle copies patched | `MTG` and `Shandalar-Win8-Test` `C:\Shandalar` copies now hash to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`; `*.before-hsection-null-patch.exe` backups preserve the original bytes before the DIB patch. |
| Patched `MTG` and `Shandalar-Win8-Test` `C:\Shandalar` launches | Wine `wscript` SendKeys drove the practical C-drive launch paths past the reported crash point; logs show `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` loaded with no original page fault/assertion. |
| Same-arrow movement-stop patch | Static disassembly verifies hooks at `0x44398c`, `0x444a2b`, and `0x444aa7`, with code cave `0x46502d`; pressing the same movement key after at least one completed movement step should route through the existing Escape stop path at `0x444a96`. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --version` | CrossOver 26.1.0 is installed locally. |
| `command -v wine` | No standalone `wine` was found in `PATH`. |
| Earlier timed CrossOver launch attempts from `Program/` in bottle `MTG` | `Shandalar.exe --help`, `Shandalar.exe`, and `Magic.exe` exited with code 53 and no capturable stdout/stderr. This is not a gameplay verification. |
| `make -n` in `src/` | Dry run reached many compile commands, then stopped because `src/card_id.h` is missing and `functions/utility.obj` had no usable rule. |

## Known Issues

| Issue | What to do |
| --- | --- |
| `Shandalar.exe --help` output was not captured locally. | See [docs/command-line.md](docs/command-line.md); test in a visible Windows/CrossOver session. |
| `Program/` CrossOver launch attempts exited code 53 in the existing `MTG` bottle. | Do not use direct `C:\Shandalar\Program\Shandalar.exe` as the primary `MTG` retest until `Program\zlib.dll` and related DLL layout are resolved. |
| Duel prompts stop accepting `Done`, `Trigger`, or `Decline` in CrossOver. | Root and `Program/` `ManalinkEh.dll` are patched for the Samite/Femeref/Kithkin damage-prevention handler that can be reached by Femeref Healer during combat. The local `MTG` bottle copies were updated with backups; visible gameplay retesting still needs to confirm the Femeref Healer blocker scenario. |
| Declared attacker mistakes are hard to undo. | Root and `Program/` `Magic.exe` now have a conservative attacker-selection undo patch, and the local `MTG` bottle copies were updated with backups. Before pressing Done, clicking an already-declared attacker should clear `STATE_ATTACKING`; unusual attack costs, attack triggers, and banding still need visible testing. See [docs/bugs/declared-attacker-undo.md](docs/bugs/declared-attacker-undo.md). |
| Start-color `WM_CREATE CreateDIBSection` assertion, name-entry glitch, and map movement stop | FaceMaker/no-resolution and bottle-setting fixes did not solve the original issue. Root and `Program/` `Shandalar.exe` include a narrow `CreateDIBSection` compatibility patch, a default-name seed plus name-editor bypass/fallback, and a same-arrow movement-stop patch; the active FaceMaker copies have the same `hSection = NULL` patch at their own DIB wrapper. Direct patched FaceMaker startup is verified; full Shandalar-spawned character creation and movement control still need a visible manual retest. See [docs/troubleshooting.md](docs/troubleshooting.md), [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md), and [docs/adventure-map-movement.md](docs/adventure-map-movement.md). |
| Runtime advice is mixed in old docs. | Use [docs/runtime-dependencies.md](docs/runtime-dependencies.md), which separates import evidence from historical notes. |
| Antivirus/security state is unknown. | No scanner was run in this pass. See [docs/security-scan.md](docs/security-scan.md) before making safety claims. |
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
| Security scan | Run a named scanner and record hashes/results. |

## More Docs

| Doc | Use it for |
| --- | --- |
| [docs/running.md](docs/running.md) | Windows, CrossOver, Wine launch commands and test matrix. |
| [docs/share-readiness.md](docs/share-readiness.md) | What is ready to share and what still needs proof. |
| [docs/crossover-macos.md](docs/crossover-macos.md) | Practical CrossOver bottle setup and troubleshooting. |
| [docs/magic-exe.md](docs/magic-exe.md) | Dedicated `Magic.exe` notes, imports, hypotheses, and tests. |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Practical troubleshooting entries, including the start-color DIB assertion. |
| [docs/bugs/create-dibsection-after-color.md](docs/bugs/create-dibsection-after-color.md) | Focused investigation of the `WM_CREATE CreateDIBSection` assertion after choosing a start color. |
| [docs/bugs/duel-freeze-damage-prevention.md](docs/bugs/duel-freeze-damage-prevention.md) | Focused investigation of the Femeref/Samite/Kithkin damage-prevention activation freeze. |
| [docs/bugs/declared-attacker-undo.md](docs/bugs/declared-attacker-undo.md) | Focused notes for the `Magic.exe` attacker-selection undo patch. |
| [docs/magic-vs-shandalar-runtime.md](docs/magic-vs-shandalar-runtime.md) | Runtime comparison notes for `Magic.exe` and `Shandalar.exe`. |
| [docs/runtime-dependencies.md](docs/runtime-dependencies.md) | PE inspection and runtime dependency matrix. |
| [docs/security-scan.md](docs/security-scan.md) | Antivirus/security scan guidance and reporting template. |
| [docs/command-line.md](docs/command-line.md) | `--help`, `--e`, `--p`, and command-line evidence. |
| [docs/save-state.md](docs/save-state.md) | Tracked save slots, screen names, and cleanup plan. |
| [docs/cleanup-move-plan.md](docs/cleanup-move-plan.md) | Exact pending cleanup moves that need explicit approval or launch-copy testing. |
| [docs/architecture.md](docs/architecture.md) | High-level repo organization. |
| [docs/building.md](docs/building.md) | What appears buildable and current blockers. |
| [docs/file-inventory.md](docs/file-inventory.md) | Counts, large files, duplicate observations, and runtime-critical file types. |
| [docs/cleanup-audit.md](docs/cleanup-audit.md) | Cleanup candidates with confidence and evidence. |
| [docs/stale-references.md](docs/stale-references.md) | Old URLs, hard-coded paths, and historical references. |
| [docs/reorganization.md](docs/reorganization.md) | Exact limited-reorg moves, rationale, commands, and verification. |
| [docs/gaps.md](docs/gaps.md) | Remaining launch, command-line, duplicate, and build gaps. |
| [docs/adventure-map-movement.md](docs/adventure-map-movement.md) | Static notes and verification commands for the same-arrow adventure-map stop patch. |
| [docs/generated/README.md](docs/generated/README.md) | Map for generated evidence snapshots. |
