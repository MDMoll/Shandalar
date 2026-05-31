# Verified on This Machine

This file keeps the detailed local command/result evidence out of the README.
It records what was actually checked on this checkout; it is not a substitute
for manual gameplay testing.

Local checkout path: `/Users/mdmoll/Shandalar/Shandalar`

## Command Evidence

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
| `sed -n '790,825p;965,970p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"` | Bottle `MTG` currently sets `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` to app-default `Version=win7` and desktop `Shandalar1440`; `Shandalar1440` is `1440x1080`. |
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
| `tools/verify-crossover-mtg-state.sh` | Optional local verifier passes for bottle `MTG`: copied runtime hashes match the docs, FaceMaker support files are present, both copied Shandalar ini files use `Window = 2`, app-default `win7` and `Shandalar1440=1440x1080` are present, and the paging-file registry setting is present. |
| `command -v wine` | No standalone `wine` was found in `PATH`. |
| `xprotect version` | macOS XProtect reports `Version: 5346 Installed: 2026-05-28 08:53:39 +0000`. This is metadata only, not a repo file scan. |
| `xprotect status` | `XProtect launch scans: disabled`; `XProtect background scans: disabled`. No XProtect result is available for the required security-scan gate. |
| `clamscan --fail-if-cvd-older-than=7 --file-list=/private/tmp/shandalar-security-scan-target-paths-6cf64837.txt --log=/private/tmp/shandalar-clamscan-6cf64837.log` | ClamAV `1.5.2/28017/Sun May 31 02:27:13 2026` scanned 241 tracked security targets and reported `Infected files: 0`; `tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all` validated 241 local TSV rows. |
| Earlier timed CrossOver launch attempts from `Program/` in bottle `MTG` | `Shandalar.exe --help`, `Shandalar.exe`, and `Magic.exe` exited with code 53 and no capturable stdout/stderr. This is not a gameplay verification. |
| `make -n` in `src/` | Dry run reached many compile commands, then stopped because `src/card_id.h` is missing and `functions/utility.obj` had no usable rule. |

## Automated Subset

Run the automated subset of these checks with:

```sh
tools/verify-share-readiness.sh
```

That script checks the clean tree, expected tracked ignored file, Git binary
attributes, patched runtime hashes, representative patch bytes, core docs, and
local Markdown links. It does not launch the game.
