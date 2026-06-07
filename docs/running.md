# Running Shandalar and Magic.exe

Verified checkout path: `/Users/mdmoll/Shandalar/Shandalar`

Runtime testing policy: use [runtime-testing-policy.md](runtime-testing-policy.md)
before launching Wine/CrossOver from an agent session. One bounded launch/log
attempt is useful evidence; repeated GUI focus or SendKeys retries are not.

## Launch Inventory

| Executable/script | Purpose | Working directory | Nearby files to preserve | Verified status | Notes |
| --- | --- | --- | --- | --- | --- |
| `Shandalar.exe` | Shandalar adventure shell and duel entry path. | repo root or `C:\Shandalar` in copied bottle | Root DLLs, `Shandalar.ini`, `Cards.dat`, `DBInfo.dat`, `Rarity.dat`, art/sound/resource folders, root `Magic.exe`. | Present, PE32 i386, and patched for the start-color `CreateDIBSection` crash, default-name seed/bypass/fallback, same-arrow movement stop, and WinMM timer-callback compatibility. In `Shandalar-Win8-Test`, the hSection patch passed the reported crash point; the broader prompt/button stability patch still needs visible manual verification. | Same SHA-256 as `Program/Shandalar.exe`; local copied bottle installs were patched with backups. |
| `Magic.exe` | Duel executable opened by root `Shandalar.exe`. | repo root or `C:\Shandalar` in copied bottle | Root Manalink DLLs, card/art/deck/sound folders. | Present and PE32 i386. Opened by logged root Shandalar startup and by a bounded `--e 0442 --p 0442` direct-duel run. Patched for declared-attacker undo and missing-registry `ShowCoinFlips` default-off behavior; root `ManalinkEh.dll` is patched for Samite-family and generic activated damage-prevention gating, AI decision-time clamping, AI raw-mana snapshot restore safety, Piranha Marsh/Bojuka Bog trigger targeting, generic AI player-only target selection, and AI land CIP resolver stack-bypass handling. | Different SHA-256 from `Program/Magic.exe`; test separately. |
| `Program/Shandalar.exe` | Alternate Shandalar copy. | `Program/` | `Shandalar.dll`, `Shandalar.ini`, `Manalink.ini`, `Cards.dat`, `DBInfo.dat`, `Rarity.dat`, art/sound/resource folders, imported DLLs including `zlib.dll`, Program helper DLLs, `libgcc_s_dw2-1.dll`, Program `DuelArt` configs, Program `TT*.ttf` drawcard fonts, and `CardArt` drawcardlib assets. | Present, PE32 i386, and patched the same way as root `Shandalar.exe`, including the WinMM timer-callback compatibility patch. `Program/zlib.dll`, Program helper DLLs, `Program/libgcc_s_dw2-1.dll`, adjacent Program config/font/card-data files, and the Program CardArt assets observed so far are now present in this checkout and in the local `MTG` copied install; the latest bounded log loaded the Program helper DLLs and card-data files without the earlier fatal strings, but the copied `MTG` Program path still needs visible retesting. | Same SHA-256 as root `Shandalar.exe`; direct Program-path support depends on adjacent Program runtime files. |
| `Program/Magic.exe` | Manalink / launcher duel executable. | `Program/` | `ManalinkEh.dll`, `ManalinkEx.dll`, `Deckdll.dll`, `Drawcardlib.dll`, `CardArtLib.dll`, art/deck/sound folders. | Present and PE32 i386. Patched for declared-attacker undo and missing-registry `ShowCoinFlips` default-off behavior; `Program/ManalinkEh.dll` has the same damage-prevention, AI clamp, AI raw-mana snapshot, Piranha Marsh/Bojuka Bog trigger-target, generic AI player-target selector, and AI land CIP resolver stack-bypass handling patches at its own offsets. Earlier CrossOver run from `Program/` exited 53 with no visible result. | Treat as a first-class target. |
| `FaceMaker.exe` / `FaceMaker-nores.exe` | Character portrait/name helper family used by Shandalar new-game setup. | Same folder as the active Shandalar launch | `FaceData.txt`, `FaceButtons.txt`, `FaceArt/` or `Faceart/`, `PlayFace/`, `Faces/` | Present. Active `FaceMaker.exe` copies are patched from the no-resolution/Korath helper; direct patched root FaceMaker launch in bottle `MTG` rendered without the DIB assertion. A later visible S2 run logged `FaceMaker-nores.exe /S`. | Preserve both active and no-resolution helper names until character creation is fully understood. |
| `Manalink_Launcher.cmd` | Windows batch menu for Manalink/mods. | repo root | `Program/`, `Mods/`, `PlayDeckAnalyser/` | Inspected only. | Lines 7-10 set `mlDir=Program`; lines 100-102 run `Magic.exe`. |
| `Shandalar help.bat` | Existing CLI example. | likely folder containing `Shandalar.exe` | `Shandalar.exe` and runtime assets. | Inspected only. | Contains `shandalar --e 0442 --p 0442`; meaning needs testing. |
| `archive/local-helpers/shandalar_homedoom.bat` | Local machine helper preserved from old root path `shandalar_homedoom.bat`. | hard-coded `e:\Program Files\Magic` | `RC.exe` | Stale/local; archived in limited reorg. | Not suitable as general launch documentation. |

## Native Windows Quick Start

1. Clone or download the whole repository. Do not copy only the `.exe` files.
2. Start with the root Shandalar executable from the full checkout:

```bat
cd \path\to\Shandalar
Shandalar.exe
```

3. If new-game setup fails after choosing a color, confirm this helper and its
   data exist:

```bat
dir FaceMaker.exe
dir FaceData.txt
dir FaceButtons.txt
dir FaceArt
```

4. Test root `Magic.exe` separately, then test the Manalink `Program` copy:

```bat
cd \path\to\Shandalar
Magic.exe

cd \path\to\Shandalar\Program
Magic.exe
```

5. If using the launcher menu, run from the repo root:

```bat
cd \path\to\Shandalar
Manalink_Launcher.cmd
```

## CrossOver on macOS Quick Start

Create a new 32-bit bottle first. For this checkout, a real Windows 8 test
bottle exists as `Shandalar-Win8-Test`. The patched `MTG` and
`Shandalar-Win8-Test` `C:\Shandalar\Shandalar.exe` launch paths have both
passed the reported start-color crash point.

The existing local `MTG` bottle has a copied install at `C:\Shandalar`. Its
shortcut targets root `C:\Shandalar\Shandalar.exe`. That copied executable was
patched in this pass, with a backup saved beside it. Direct
`C:\Shandalar\Program\Shandalar.exe` previously failed before gameplay in this
bottle because `Program\zlib.dll` was missing. This checkout and the local
copied bottle install now both include matching root/Program zlib DLLs. A later
visible direct-Program attempt reached `drawcardlib.dll` and reported missing
`C:\Shandalar\Program\CARDART\ManaSymbols.pic`; the repo and copied install now
also have the Program CardArt files recorded in
[runtime-manifest.md](runtime-manifest.md). A bounded exact-path log after those
copies reached missing `C:\Shandalar\Program\CARDART\modern\Triggering.png`;
a follow-up bounded log after that copy loaded `Triggering.png` and reached
missing `C:\Shandalar\Program\CARDART\planeswalker\LoyaltyBase.png`; a
2026-06-04 bounded log after the loyalty copy reached missing
`C:\Shandalar\Program\CARDART\modern\CardOv_Nyx.png`. Follow-up logs after
that copy reached missing `C:\Shandalar\Program\Manalink.ini`, then missing
`C:\Shandalar\Program\DuelArt\Modern.dat`, then missing
`C:\Shandalar\Program\DuelArt\Planeswalker.dat` and six Program `TT*.ttf`
font paths. The four Planeswalker loyalty images, generic Modern Nyx overlay,
adjacent Program config files, and Program fonts are now present too. A later
visible fatal reported `Bad raw_cards_data[-1].card() building Hornet:
expected 15835, got -1`; the older Program `Cards.dat`/`Rarity.dat` files had
15718 card records, while the root files have 16818. After those were copied,
DeckDLL reported `Fatal: Couldn't find Cards.dat, DBInfo.dat or Rarity.dat`
because the older Program `DBInfo.dat` still had 15718 records. `Program/Cards.dat`,
`Program/DBInfo.dat`, and `Program/Rarity.dat` now match the root trio in the
checkout and the local copied install. A later visible recurrence of the
Hornet fatal came from stale Program helper DLLs rather than the card-data trio:
`Program/Shandalar.dll`, `Program/CardArtLib.dll`, `Program/Deckdll.dll`, and
`Program/Drawcardlib.dll` now match root, and the newer Program drawcard helper
also has adjacent `Program/libgcc_s_dw2-1.dll`. The latest bounded log loaded
those Program helper DLLs and data files without the earlier fatal strings. The
exact Program path still needs visible retesting.

Backup note: bottle-local originals were saved as
`Shandalar.before-hsection-null-patch.exe` and
`FaceMaker.before-hsection-null-patch.exe` before patching.
The local `MTG` `Magic.exe` copies also have `*.before-coinflip-default-patch.exe`
backups from the coin-flip default patch, and `user.reg` was backed up before
changing `ShowCoinFlips` to `0`.
A bounded 2026-06-04 direct-duel smoke using `C:\Shandalar\Shandalar.exe --e 0442 --p 0442`
printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"` and opened
root `C:\Shandalar\Magic.exe` without targeted coin-flip, Hornet/card-data,
page-fault, DIB-assertion, or unhandled-exception strings in
`/tmp/shandalar-mtg-root-ep0442-coinflip-default-cx.log`; the child stayed alive
until the alarm and was killed, so this is not visible gameplay proof.

Use CrossOver's Run Command for bottle `MTG`:

| Field | Value |
| --- | --- |
| Bottle | `MTG` |
| Command | `C:\Shandalar\Shandalar.exe` |
| Working directory | `C:\Shandalar` |
| Virtual desktop | `Shandalar1440`, `1440x1080` |
| App-default Windows version | `win7` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe` |
| Duel option | `ShowCoinFlips=0` |

Use CrossOver's Run Command for bottle `Shandalar-Win8-Test`:

| Field | Value |
| --- | --- |
| Bottle | `Shandalar-Win8-Test` |
| Command | `C:\Shandalar\Shandalar.exe` for the normal copied install, or `Y:\Shandalar\Shandalar\Shandalar.exe` for the repo path |
| Working directory | `Y:\Shandalar\Shandalar` or `C:\Shandalar` to match the command |
| Virtual desktop | `ShandalarTall`, `1024x800` |
| Paging file | `C:\pagefile.sys 512 1024` |

This bottle was created from CrossOver's 32-bit `win8` template. Local registry
evidence reports `ProductName=Microsoft Windows 8`, `CurrentVersion=6.2`, and
`CurrentBuild=9200`. The checkout was then copied into the bottle at
`/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar`
so the next test uses a normal `C:` install path instead of the macOS `Y:`
drive mapping.

Then repeat with:

```text
C:\Shandalar\Magic.exe
C:\Shandalar\Program\Magic.exe
```

Command-line equivalent if CrossOver's `wine` helper is available:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" "Y:\Shandalar\Shandalar\Shandalar.exe"
```

For a fresh bottle that launches directly from the macOS checkout, start with:

```sh
cd /Users/mdmoll/Shandalar/Shandalar
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME Shandalar.exe
```

## Wine Quick Start

Standalone Wine was not found in `PATH` on this machine. If it is installed on
another machine, use:

```sh
cd /path/to/repo
wine Shandalar.exe --help
wine Shandalar.exe

cd /path/to/repo/Program
wine Magic.exe
```

## Verified on this machine

| Command | Working directory | Result |
| --- | --- | --- |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --version` | repo root | CrossOver 26.1.0.39808. |
| `command -v wine` | repo root | No standalone Wine found. |
| `shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe` | repo root | Active FaceMaker copies hash to `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246`; no-resolution/Korath helper copies hash to `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`. |
| Separate `xxd -g1 -l 32 -s $((0x5f40))` checks for `FaceMaker.exe` and `Program/FaceMaker.exe` | repo root | Both patched dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing FaceMaker's `CreateDIBSection` wrapper to receive `hSection = NULL`. |
| `cmp -s Program/FaceData.txt Manalink3/Program/FaceData.txt` | repo root | Match; FaceMaker data is present in `Program/`. |
| `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini` | repo root | Both active configs should show `Window = 2` for the current CrossOver start-color test. |
| `diff -qr Program/FaceArt Manalink3/Program/FaceArt` | repo root | No differences. |
| `shasum -a 256 Shandalar.exe Program/Shandalar.exe` | repo root | Match; root and `Program/` Shandalar binaries hash to `92cca05b493c28f6c29c0cc4bd0018499acd9a8cbdce06f9230da59d5be0a0ef`. |
| `shasum -a 256 zlib.dll Program/zlib.dll` | repo root | Match; both adjacent zlib DLLs hash to `9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90`. |
| `shasum -a 256 Manalink.ini Program/Manalink.ini Program/DuelArt/Modern.dat Program/DuelArt/Planeswalker.dat Program/TT*.ttf` | repo root | Active root and `Program/` `Manalink.ini` match at `30153fd22c76b0c0751c538938af46fbf25b1b51d5b4bb2bd9a2eead1b9c2f2b`; `Program/DuelArt/Modern.dat` hashes to `9a2d70be70b70ef27036a47550bc0d549437df0c032a4e0237a217e4731e1aee`; `Program/DuelArt/Planeswalker.dat` hashes to `619e0b9780ec204b9fbf6f48b2eb541c9d8a6f19a73f27d4d76d25828db7d369`; Program font hashes are recorded in [runtime-manifest.md](runtime-manifest.md). |
| `shasum -a 256` on the Program CardArt files listed in [runtime-manifest.md](runtime-manifest.md) | repo root | Program drawcardlib/CardArt assets are present through `Modern/CardOv_Nyx.png`, with exact hashes recorded in the runtime manifest. |
| `xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `6a 00 57 50 8b 4d 10 51 ff 75 04`, forcing `CreateDIBSection` to receive `hSection = NULL`. |
| `xxd -g1 -l 40 -s $((0xa1a42)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `c7 05 28 12 59 00 6d 50 6c 61`, seeding the name buffer with `mPlayer` before the existing gender/name handling code runs. |
| `xxd -g1 -l 32 -s $((0xa1acd)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps begin `31 c0 89 85 a8 fe ff ff`, bypassing the fragile manual name editor. |
| `xxd -g1 -l 64 -s $((0x64570)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps contain the empty-name fallback that writes `Player` before copying to `0x7a0770`. |
| Focused `lldb` checks for `0x44398c`, `0x444a2b`, `0x444aa7`, and `0x46502d` | repo root | Static verification shows movement hooks at `0x44398c`, `0x444a2b`, and `0x444aa7`, with code cave `0x46502d` and flag `0x583a2c` for same-arrow stop behavior. GNU `objdump -D -Mintel` is also usable when installed. |
| `xxd -p -l 5 -s $((0xcdd3f)) Shandalar.exe` and `Program/Shandalar.exe` | repo root | Both patched dumps are `9090909090`, NOPing the WinMM callback-thread `call 0x56d476` while preserving the callback's 33 ms tick counter increment. |
| `shasum -a 256 Magic.exe Program/Magic.exe` | repo root | Differ after both Magic patches: root `93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0`, Program `685669692634ec830fe228904e11b1b536bd4b20e52192863a6280c2dbff6b66`; test both exact paths. |
| `xxd -g1 -l 16 -s $((0x43c303-0x400000)) Magic.exe` and the same command for `Program/Magic.exe` | repo root | Both begin `e9 c0 d8 01 00 90 90 90 90 90 90 90 90` after the declared-attacker undo patch. |
| `xxd -g1 -l 80 -s $((0x459bc8-0x400000)) Magic.exe` and the same command for `Program/Magic.exe` | repo root | Both caves begin `f7 46 08 04 00 00 00 0f 85 12 00 00 00`. |
| `xxd -g1 -l 10 -s $((0x5db1f)) Magic.exe` and the same command for `Program/Magic.exe` | repo root | Both begin `c7 05 5c 72 78 00 00 00 00 00`, changing the missing-registry `ShowCoinFlips` default to off. |
| `/usr/bin/perl -e 'alarm 25; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-root-ep0442-coinflip-default-cx.log --debugmsg +seh,+file,+process "C:\Shandalar\Shandalar.exe" --e 0442 --p 0442` | repo root | Printed `Stand-alone duel: "decks/0442.dck" vs. "decks/0442.dck"`, opened root `C:\Shandalar\Magic.exe`, loaded AVI support, and opened `statwin\water.avi`. Targeted scans found no `COINTOSS`, `ShowCoinFlips`, `DIALOG_*COINFLIP`, Hornet/card-data fatal, page fault, DIB assertion, or unhandled-exception strings. The child process stayed alive until the 25-second alarm and was killed manually. |
| `shasum -a 256 ManalinkEh.dll Program/ManalinkEh.dll` | repo root | Root patched hash is `68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5`; `Program/` patched hash is `619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc`. |
| `xxd -g1 -l 32 -s $((0x3bb035)) ManalinkEh.dll` and `xxd -g1 -l 32 -s $((0x381a25)) Program/ManalinkEh.dll` | repo root | Both patched dumps begin `f6 05 90 f1 4e 00 04 0f 84 ae 00 00 00 e9 01 00`, gating the shared Samite/Femeref/Kithkin healer handler on `LCBP_DAMAGE_PREVENTION`. |
| `xxd -g1 -l 5 -s $((0x44cb23)) ManalinkEh.dll`; `xxd -g1 -l 38 -s $((0x495a30)) ManalinkEh.dll`; matching Program offsets `0x40f115` and `0x452c30` | repo root | Root and Program generic activated damage-prevention helpers jump into caves that test `GAA_DAMAGE_PREVENTION*` and `LCBP_DAMAGE_PREVENTION` before replaying the original target-check branch. |
| `xxd` checks at `0x40d0e1`/`0x495a60` for root `ManalinkEh.dll` and `0x3d2da1`/`0x452c60` for `Program/ManalinkEh.dll` | repo root | `_check_timer_for_ai_speculation` jumps to a cave that preserves configured values `1..270` and writes `270` for missing, invalid, or higher values. |
| `xxd` checks at `0x40db84`/`0x495a90` for root `ManalinkEh.dll` and `0x3d3844`/`0x452c90` for `Program/ManalinkEh.dll` | repo root | `_ai_decision_phase` jumps to a cave that saves all 16 `raw_mana_available` dwords before temporarily replacing only the opponent row during AI speculation. |
| `xxd -p -l 75 -s $((0x3fe7a0)) ManalinkEh.dll` and `xxd -p -l 75 -s $((0x3c4930)) Program/ManalinkEh.dll` | repo root | Piranha Marsh's ETB trigger calls `pick_player_duh()` so AI/Duh mode targets the opponent without opening the generic player-target prompt. |
| `xxd -p -l 117 -s $((0x3f63e0)) ManalinkEh.dll` and `xxd -p -l 117 -s $((0x3bc630)) Program/ManalinkEh.dll` | repo root | Bojuka Bog's resolving ETB trigger calls `pick_player_duh()`; manual gameplay showed this selector-side mitigation alone was not sufficient. |
| `xxd -p` checks at `0x429acf`/`0x495b00` for root `ManalinkEh.dll` and `0x3ec7cf`/`0x452d00` for `Program/ManalinkEh.dll`, plus restored Piranha/Bojuka callsites | repo root | Non-speculating AI-owned land `TRIGGER_COMES_INTO_PLAY` contexts resolve directly before Spell Chain insertion. Fresh manual Piranha/Bojuka proof is still pending. |
| `/usr/bin/perl -e 'alarm 18; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\Shandalar.exe"` | repo root | Stayed alive until the alarm; log showed app-default virtual desktop/`Version=win8`, `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-mtg-win8-appdefault-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "C:\Shandalar\FaceMaker.exe"` | repo root | Stayed alive until the alarm; log showed app-default virtual desktop/`Version=win8`, `wined3d`, `explorer.exe /desktop`, and 1024x768 8bpp DIB sections. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-direct-after-patch-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\FaceMaker.exe"` | repo root | Direct patched FaceMaker startup rendered its UI; log had no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/cxbottle --bottle Shandalar-Win8-Test --create --template win8 --description "Shandalar Win8 test bottle"` | repo root | Created a fresh 32-bit Win8 bottle. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-win8test-startup-cx.log --debugmsg +seh,+gdi,+bitmap,+loaddll,+file "Y:\Shandalar\Shandalar\Shandalar.exe"` | repo root | Stayed alive until cleanup after the timed smoke; log showed `wined3d`, successful `NtGdiCreateDIBSection` calls, and no startup page fault. |
| `cp -cR . "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar"` | repo root | Created a bottle-local install copy for `C:\Shandalar`; `.git/fsmonitor--daemon.ipc` was a socket and was not copied. |
| `/usr/bin/perl -e 'alarm 12; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-tall-startup-cx.log --debugmsg +seh,+bitmap,+process "C:\Shandalar\Shandalar.exe"` | repo root | Stayed alive until cleanup after the timed smoke; log used the bottle-local `C:\Shandalar` path and showed no startup page fault. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" --cx-log /tmp/shandalar-repo-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "Y:\Shandalar\Shandalar\Shandalar.exe"` plus `wscript "Y:\Shandalar\Shandalar\local\crossover\sendkeys-start-color.vbs"` | repo root | Passed the reported crash point; log showed post-color loads of `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`, with no original page fault/assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" --cx-log /tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"` plus the same `wscript` helper | repo root | Passed the reported crash point from the normal C-drive install path; log showed the same post-color resource loads with no original page fault/assertion. |
| `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\Shandalar.exe"` plus the same `wscript` helper | repo root | Passed the reported crash point from the older practical `MTG` shortcut path; log showed the same post-color resource loads with no original page fault/assertion. |
| Visual smoke launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | repo root | Reached the `Magic: Shandalar` main menu; macOS denied synthetic keypresses, so Start New Game was not tested. |
| Visible SendKeys run of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | repo root | Reached the adventure map for the default/first start-color path; see [manual-gameplay-verification.md](manual-gameplay-verification.md) S2 and [generated/manual-gameplay/s2-map-2026-05-31.md](generated/manual-gameplay/s2-map-2026-05-31.md). Raw local log `/tmp/shandalar-visible-s2-attempt-cx.log` observed `FaceMaker-nores.exe /S`. |
| Earlier direct logged launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` | repo root | Loader failure before gameplay because `Program\zlib.dll` was missing, cascading through `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll`. |
| Bounded logged launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the zlib layout fix | repo root | `/tmp/shandalar-mtg-program-zlib-retest-cx.log` loaded Program `zlib.dll`, `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll` natively and a targeted fatal-error scan found no old loader/import/page-fault pattern. This was a timed GUI smoke, not a visible gameplay pass. |
| Visible direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the zlib layout fix | repo root | Stopped at a `drawcardlib.dll` dialog: `Could not load ManaSymbols= "C:\Shandalar\Program\CARDART\ManaSymbols.pic"`. The repo and local copied install now contain the Program CardArt, config, font, and card-data files listed in [runtime-manifest.md](runtime-manifest.md); visible exact-path retest is still pending after the latest visible fatals exposed the older Program card-data trio. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the first Program CardArt copy | repo root | `/tmp/shandalar-mtg-program-cardart-retest-cx.log` loaded the first Program CardArt files, then reached a missing `C:\Shandalar\Program\CARDART\modern\Triggering.png` lookup. That file is now copied into the repo and local copied install. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the `Triggering.png` copy | repo root | `/tmp/shandalar-mtg-program-cardart-triggering-retest-cx.log` loaded `C:\Shandalar\Program\CARDART\modern\Triggering.png`, then reached a missing `C:\Shandalar\Program\CARDART\planeswalker\LoyaltyBase.png` lookup. The four Planeswalker loyalty images are now copied into the repo and local copied install; no further visible retest has been run. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the loyalty-image copy | repo root | `/tmp/shandalar-mtg-program-cardart-loyalty-retest-cx.log` loaded the loyalty-image fixes, then reached a missing `C:\Shandalar\Program\CARDART\modern\CardOv_Nyx.png` lookup. `Program/DuelArt/Duel.dat` maps the Modern `CardOv_*Nyx` frame entries to this generic overlay; `Modern/CardOv_Nyx.png` is now copied into the repo and local copied install. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the `CardOv_Nyx.png` copy | repo root | `/tmp/shandalar-mtg-program-cardart-nyx-retest-cx.log` opened the known Program CardArt files, then repeatedly looked for missing `C:\Shandalar\Program\Manalink.ini`. `Program/Manalink.ini` is now copied into the repo and local copied install. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the `Program/Manalink.ini` copy | repo root | `/tmp/shandalar-mtg-program-manalinkini-retest-cx.log` opened `C:\Shandalar\Program\Manalink.ini` and known Program CardArt files, then repeatedly looked for missing `C:\Shandalar\Program\DuelArt\Modern.dat`. `Program/DuelArt/Modern.dat` is now copied into the repo and local copied install. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the `Modern.dat` copy | repo root | `/tmp/shandalar-mtg-program-moderndat-retest-cx.log` opened `Modern.dat`, `Program\Manalink.ini`, and known Program CardArt files, then repeatedly looked for missing `C:\Shandalar\Program\DuelArt\Planeswalker.dat` and Program `TT*.ttf` font files. Those files are now copied into the repo and local copied install. |
| Bounded logged direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the `Planeswalker.dat` and font copies | repo root | `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log` opened `Program\DuelArt\Modern.dat`, `Program\DuelArt\Planeswalker.dat`, the six Program `TT*.ttf` font paths, and then `C:\Shandalar\Program\Magic.exe`. The run exited with code 1 and the targeted scan found no new `drawcardlib.dll` missing-asset dialog, page fault, fatal assertion, or new Program data/art/font missing path. |
| Visible direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the Program adjacent config/font/art copies | repo root | Stopped at a fatal dialog: `Bad raw_cards_data[-1].card() building Hornet: expected 15835, got -1`. The pre-fix Program `Cards.dat`/`Rarity.dat` files had 15718 cards, while root has 16818; those Program files were copied from root. |
| `/usr/bin/perl -e 'alarm 20; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" --cx-log /tmp/shandalar-mtg-program-carddata-retest-cx.log --debugmsg +seh,+file "C:\Shandalar\Program\Shandalar.exe"` | repo root | Opened `C:\Shandalar\Program\Cards.dat` with `ret 0`; targeted scan found no `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, or unhandled-exception strings. The next visible run reported DeckDLL's generic `Cards.dat, DBInfo.dat or Rarity.dat` fatal because Program `DBInfo.dat` still had 15718 records. |
| Visible direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the Program `Cards.dat`/`Rarity.dat` copies | repo root | Stopped at a DeckDLL fatal dialog: `Fatal: Couldn't find Cards.dat, DBInfo.dat or Rarity.dat`. Source/header inspection showed `DBInfo.dat` opened but its 15718-record header no longer matched the 16818-card Program `Cards.dat`; `Program/DBInfo.dat` now matches root in the repo and local copied install. |
| `/usr/bin/perl -e 'alarm 20; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" --cx-log /tmp/shandalar-mtg-program-dbinfo-retest-cx.log --debugmsg +seh,+file "C:\Shandalar\Program\Shandalar.exe"` | repo root | Opened `C:\Shandalar\Program\DBInfo.dat` with `ret 0`; targeted scan found no generic card-data fatal, `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, or unhandled-exception strings. The process stayed alive until the 20-second alarm and was killed; cleanup tail includes `wineserver crashed`, so this is specific-fatal evidence only. |
| `/usr/bin/perl -e 'alarm 20; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" --cx-log /tmp/shandalar-mtg-program-post-dbinfo-visible-retest-cx.log --debugmsg +seh,+file "C:\Shandalar\Program\Shandalar.exe"` | repo root | Opened `C:\Shandalar\Program\Cards.dat`, `DBInfo.dat`, `Rarity.dat`, `shandalar.dll`, and `Shandalar.ini`; targeted scan found no generic card-data fatal, `raw_cards_data`, Hornet fatal, missing-asset dialog, page fault, assertion, or unhandled-exception strings. The child process remained alive past the 20-second alarm and was killed manually, so this is loader/fatal-regression evidence only. |
| Visible direct launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` after the Program card-data trio was present | repo root | Stopped again at the Hornet `raw_cards_data[-1]` fatal. Strings and hash inspection showed the exact assert string lived in the stale `Program/Shandalar.dll` helper generation. `Program/Shandalar.dll`, `Program/CardArtLib.dll`, `Program/Deckdll.dll`, and `Program/Drawcardlib.dll` now match root in the repo and local copied install. |
| `/usr/bin/perl -e 'alarm 20; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" --cx-log /tmp/shandalar-mtg-program-dll-sync-retest-cx.log --debugmsg +seh,+file "C:\Shandalar\Program\Shandalar.exe"` | repo root | After syncing the Program helper DLLs, the old Hornet fatal was absent, but the newer Program `DrawCardLib.dll` could not load because `C:\Shandalar\Program\libgcc_s_dw2-1.dll` was missing. `Program/libgcc_s_dw2-1.dll` now matches root in the repo and local copied install. |
| `/usr/bin/perl -e 'alarm 20; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" --cx-log /tmp/shandalar-mtg-program-dll-sync-libgcc-retest-cx.log --debugmsg +seh,+file "C:\Shandalar\Program\Shandalar.exe"` | repo root | Loaded `C:\Shandalar\Program\DrawCardLib.dll`, `libgcc_s_dw2-1.dll`, `DECKDLL.dll`, `shandalar.dll`, and the Program card-data files. Targeted scan found no generic card-data fatal, `raw_cards_data`, Hornet fatal, missing-asset dialog, missing-library import, page fault, assertion, or unhandled-exception strings. The process stayed alive until the 20-second alarm; leftover `Shandalar.exe` was killed manually, so this is loader/fatal-regression evidence only. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Shandalar.exe --help` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Shandalar.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |
| `/usr/bin/perl -e 'alarm 15; exec @ARGV' /Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG Magic.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Exit code 53, no stdout/stderr captured. |

These results prove the commands were attempted in the existing local `MTG`
bottle. They do not prove that the game fails in all CrossOver bottles.

## Test Matrix

| Platform/bottle | Executable | Expected check | Status |
| --- | --- | --- | --- |
| Windows native | Root `Shandalar.exe` | Main Shandalar UI opens, can start or load game. | Needs testing. |
| Windows native | Root `Magic.exe` | Duel shell opens or reports a visible dependency/config issue. | Needs testing. |
| Windows native | `Program/Magic.exe` | Manalink/duel shell opens. | Needs testing. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | S1/S2 visible evidence reaches main menu and the default/first start-color adventure map; remaining colors, movement, save/load, and duel rows still need manual testing. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\Program\Shandalar.exe` | Alternate Program Shandalar path opens from `C:\Shandalar\Program` after adjacent DLLs/assets/config/fonts/card-data are present. | Loader-path smoke no longer shows the old zlib cascade; later Program CardArt, config, font, card-data, helper-DLL, and `libgcc_s_dw2-1.dll` gaps have been copied and hash-checked. Latest bounded log loaded the Program helper DLLs and card-data files without the earlier fatal strings. Exact-path visible retest still needed. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\FaceMaker.exe` and preserved `C:\Shandalar\FaceMaker-nores.exe` | Character helper opens and creates its DIB surfaces. | Direct patched helper launch rendered successfully; the S2 run also observed `FaceMaker-nores.exe /S`, so both helper names remain protected until the remaining character-creation color paths are tested. |
| CrossOver `Shandalar-Win8-Test` bottle | patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | Automated SendKeys smoke passed the reported crash point before the name-entry and movement patches; full visible gameplay still needs manual testing. |
| CrossOver `Shandalar-Win8-Test` bottle | patched `C:\Shandalar\Shandalar.exe` | New game reaches character creation/map after choosing color, with default `Player` name and same-arrow map stop. | Automated SendKeys smoke passed the reported crash point from the normal C-drive install path before the name-entry and movement patches; full visible gameplay still needs manual testing. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\Magic.exe` | Root duel executable opens; duel-start coin-flip animation is skipped or otherwise does not freeze before play starts. | Static repo and copied-bottle patch bytes match, `MTG` `ShowCoinFlips=0` is verified, and a bounded `--e 0442 --p 0442` log opened root `Magic.exe` without targeted coin-flip or fatal strings. Needs visible testing. |
| CrossOver `MTG` bottle | patched root `C:\Shandalar\ManalinkEh.dll` with `C:\Shandalar\Magic.exe` | Samite-family and generic activated damage-prevention abilities should only appear during the engine damage-prevention window; `_check_timer_for_ai_speculation` clamps missing, invalid, or high `AiDecisionTime` values to 270; `_ai_decision_phase` saves both players' raw-mana rows before speculation temporarily replaces the opponent row; non-speculating AI-owned land ETB triggers should resolve before Spell Chain insertion. | Repo and copied `MTG` Manalink DLLs now match; visible Bojuka/Piranha retest still needed. |
| CrossOver `MTG` bottle | patched root `C:\Shandalar\Magic.exe` | During declare attackers, clicking an already-declared ordinary attacker before Done removes it from the declared attackers selection. | Repo root and copied `MTG` bottle `Magic.exe` are patched; visible retest still needed. |
| CrossOver `MTG` bottle | patched `C:\Shandalar\Program\Magic.exe` | Program Manalink executable opens; missing `ShowCoinFlips` registry data would default animations off. | Static repo and copied-bottle patch bytes match; needs visible testing. |
| CrossOver XP bottle | root `Shandalar.exe` from copied install or repo root | UI opens, start-color flow works. | Needs testing. |
| CrossOver XP bottle | `Program/Magic.exe` | UI opens from `Program/` working directory. | Needs testing. |
| CrossOver Windows 7 bottle | both | Compare to XP. | Needs testing. |
| CrossOver Windows 8 bottle | both | Compare to the forum thread's Windows 8 success clue. | `Shandalar-Win8-Test` created; patched repo Shandalar passed the crash-point smoke test. |
| CrossOver Windows 10 bottle | both | Try if XP/7 do not work. | Needs testing. |
| CrossOver Windows 11 bottle | both | Comparison/fallback only. | Needs testing. |

## Capturing Logs

| Environment | Practical capture method |
| --- | --- |
| CrossOver GUI | Use Run Command, enable logging if offered, and save the log path with bottle name and Windows version. |
| CrossOver CLI | Run from Terminal and redirect output outside the repo if needed: `/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle BOTTLE_NAME Magic.exe >~/Desktop/magic.log 2>&1`. |
| Wine CLI | Run with Wine debug channels only when needed: `WINEDEBUG=+loaddll,+seh wine Magic.exe`. |
| Windows | Check Event Viewer only if the app exits silently. Also note any missing DLL dialog exactly. |

Follow [runtime-testing-policy.md](runtime-testing-policy.md): run once for a
targeted question, save the command/log/screenshot/result, then stop if focus,
keystrokes, or display mode are unreliable.

## Needs testing

`Shandalar.exe --help` may show a dialog, write to a log, or require a console
wrapper. This pass did not capture help text.
