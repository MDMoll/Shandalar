# Runtime Manifest

Generated from local inspection on 2026-05-31 and updated on 2026-06-08 in
`/Users/mdmoll/Shandalar/Shandalar`.

This manifest is for identity, review, and scan handoff. It is not a malware
scan, a license grant, or proof of gameplay stability.

## Active Patched Runtime Files

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` | PE32 GUI Intel 80386 | Root CrossOver launch target; patched for DIB, name, movement, minimal WinMM timer-callback compatibility, MagSnd update-message callback compatibility, MagSnd initialization-disable behavior, and MCIWndCreateA disable behavior. |
| `Program/Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` | PE32 GUI Intel 80386 | Same bytes as root, including the minimal WinMM timer-callback, MagSnd update-message, MagSnd initialization-disable, and MCIWndCreateA disable compatibility patches; adjacent `Program/zlib.dll`, Program helper DLLs, `Program/libgcc_s_dw2-1.dll`, Program config, font, card-data, and CardArt assets observed so far are now present in this checkout and the local copied install, but the direct copied-bottle Program path still needs visible retesting. |
| `Magic.exe` | `93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0` | PE32 GUI Intel 80386 | Root duel executable; patched for declared-attacker undo and `ShowCoinFlips` default-off behavior. |
| `Program/Magic.exe` | `685669692634ec830fe228904e11b1b536bd4b20e52192863a6280c2dbff6b66` | PE32 GUI Intel 80386 | Manalink launcher target; patched for declared-attacker undo and `ShowCoinFlips` default-off behavior. |
| `FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | PE32 GUI Intel 80386 | Active patched FaceMaker helper. |
| `Program/FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | PE32 GUI Intel 80386 | Same bytes as root active FaceMaker helper. |
| `ManalinkEh.dll` | `68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5` | PE32 DLL Intel 80386 | Root DLL patched for Samite-family damage-prevention activation, generic activated damage-prevention gating, AI decision-time clamping, AI raw-mana speculation snapshot restore safety, Piranha Marsh/Bojuka Bog AI trigger targeting, generic AI player-only target selection, and AI land CIP resolver stack-bypass land handling. |
| `Program/ManalinkEh.dll` | `619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc` | PE32 DLL Intel 80386 | Program DLL patched at its own offsets for Samite-family damage-prevention activation, generic activated damage-prevention gating, AI clamping, AI raw-mana speculation snapshot restore safety, Piranha Marsh/Bojuka Bog AI trigger targeting, generic AI player-only target selection, and AI land CIP resolver stack-bypass land handling. |
| `zlib.dll` | `9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90` | PE32 DLL Intel 80386 | Root image/decompression support DLL. |
| `Program/zlib.dll` | `9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `zlib.dll` added to close the adjacent Program loader gap in this checkout. |
| `Shandalar.dll` | `f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7` | PE32 DLL Intel 80386 | Root Shandalar helper generation; contains `.cdxai` hooks for the AI land CIP resolver stack-bypass at `0x94d34`/`0x1174800` and the AI player-only target selector at `0xcb16`/`0x1174920`. |
| `Program/Shandalar.dll` | `f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `Shandalar.dll`; includes the same `.cdxai` AI land CIP resolver and AI player-only target selector patches. |
| `CardArtLib.dll` | `c1a68591059ff3e650104bf711d4e3f0c9a01a232db2e594af64aaa6846b3c1d` | PE32 DLL Intel 80386 | Root card-art helper rebuilt so GDI+ starts lazily outside `DllMain`, uses explicit notification hook/unhook handling, suppresses external codecs, validates image load status before caching, and keeps PE `DllCharacteristics` `0x0000` with `___ImageBase=__image_base__`. |
| `Program/CardArtLib.dll` | `c1a68591059ff3e650104bf711d4e3f0c9a01a232db2e594af64aaa6846b3c1d` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `CardArtLib.dll` for direct Program-path Shandalar launches. |
| `DeckDLL.dll` | `5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0` | PE32 DLL Intel 80386 | Root deck/card-data helper rebuilt from `src/deck` with the Shandalar minimum-deck-size guard and legacy-safe startup flags: no dynamic base/NX compatibility bits, plus `___ImageBase=__image_base__`. |
| `Program/Deckdll.dll` | `5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `DeckDLL.dll` with the existing Program filename case preserved. |
| `Drawcardlib.dll` | `24fbf84f26dd8258cd9ae23350971080fa2f192a210f610ca6a135b669b6ead9` | PE32 DLL Intel 80386 | Root card-rendering helper generation rebuilt so Drawcardlib suppresses GDI+ external codecs, uses explicit notification hooks instead of allowing a GDI+ background thread, checks hook status, routes high-frequency GDI+ creation/clone/lock/draw/dimension calls through checked wrappers, and keeps legacy-safe PE startup flags with `DllCharacteristics` `0x0000`. |
| `Program/Drawcardlib.dll` | `24fbf84f26dd8258cd9ae23350971080fa2f192a210f610ca6a135b669b6ead9` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `Drawcardlib.dll`; this generation imports Wine-provided `api-ms-win-crt-*` DLLs instead of `libgcc_s_dw2-1.dll` directly and keeps `DYNAMIC_BASE`/`NX_COMPAT` disabled for CrossOver loader compatibility. |
| `Statwin.dll` | `f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf` | PE32 DLL Intel 80386 | Root status/video-adjacent helper; patched at VA `0x10003610` / file offset `0x2a10` so the dynamic MagVid loader returns video-unavailable before loading `magvid.dll`. |
| `Program/Statwin.dll` | `f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `Statwin.dll`; includes the same MagVid loader-disable compatibility patch. |
| `libgcc_s_dw2-1.dll` | `89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b` | PE32 DLL Intel 80386 | Preserved GCC runtime helper from the previous drawcard helper generation and Program-path loader fix. |
| `Program/libgcc_s_dw2-1.dll` | `89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b` | PE32 DLL Intel 80386 | Byte-for-byte copy of root `libgcc_s_dw2-1.dll` kept for compatibility with previous helper generations and the documented Program-path loader fix. |

## Active Card Data Files

`Cards.dat`, `DBInfo.dat`, and `Rarity.dat` are working-directory-relative
runtime data. Keep each folder's trio in sync: `DeckDLL` checks that
`DBInfo.dat` and `Rarity.dat` record counts match the loaded `Cards.dat` count,
and direct Program-path Shandalar startup now expects card ids beyond the older
Program trio.

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Cards.dat` | `abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94` | data | Root active card database; header count is `0x41b2` / 16818 cards. |
| `Program/Cards.dat` | `abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94` | data | Byte-for-byte copy of root `Cards.dat`; replaced the older Program copy whose 15718-card header made `raw_cards_data[-1]` possible while building Hornet. |
| `DBInfo.dat` | `519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1` | data | Root active card metadata; header count is `0x41b2` / 16818 records. |
| `Program/DBInfo.dat` | `519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1` | data | Byte-for-byte copy of root `DBInfo.dat`; copied after the older 15718-record Program copy caused DeckDLL's generic `Cards.dat, DBInfo.dat or Rarity.dat` fatal. |
| `Rarity.dat` | `e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f` | data | Root active rarity/card metadata; header count is `0x41b2` / 16818 records. |
| `Program/Rarity.dat` | `e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f` | data | Byte-for-byte copy of root `Rarity.dat`; copied together with `Program/Cards.dat` to keep the Program trio internally consistent. |

## Active Program Adjacent Config

These files are required by logged direct Program-path runs of
`C:\Shandalar\Program\Shandalar.exe` from `C:\Shandalar\Program`. The local
`MTG` copied install has matching copies at the same relative paths.

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Program/Manalink.ini` | `30153fd22c76b0c0751c538938af46fbf25b1b51d5b4bb2bd9a2eead1b9c2f2b` | ASCII text | Copied from the active root `Manalink.ini`; a 2026-06-04 bounded log first reported repeated missing `C:\Shandalar\Program\Manalink.ini`, and the follow-up log opened this path successfully. |
| `Program/DuelArt/Modern.dat` | `9a2d70be70b70ef27036a47550bc0d549437df0c032a4e0237a217e4731e1aee` | ASCII text | Copied from the preserved Program-style `Mods/Art/_undo/.../DuelArt/Modern.dat` snapshot after the follow-up 2026-06-04 log repeatedly looked for `C:\Shandalar\Program\DuelArt\Modern.dat`; `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log` later opened this path with `ret 0`. |
| `Program/DuelArt/Planeswalker.dat` | `619e0b9780ec204b9fbf6f48b2eb541c9d8a6f19a73f27d4d76d25828db7d369` | ASCII text | Copied from the preserved Program-style `Mods/Art/_undo/.../DuelArt/Planeswalker.dat` snapshot after the post-`Modern.dat` 2026-06-04 log repeatedly looked for `C:\Shandalar\Program\DuelArt\Planeswalker.dat`; the follow-up log opened this path with `ret 0`. |

## Active Program Font Files

`Program/Drawcardlib.dll` builds these names from its base directory before
adding private card-rendering fonts. A post-`Modern.dat` exact-path log showed
missing `C:\Shandalar\Program\TT*.ttf` lookups for these six names. The files
below are byte-for-byte copies of the matching root font files, with names
matching the hardcoded Program-path lookups exactly.

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Program/TT0530m_.ttf` | `51afc07ba27699fec048dd387f6e6068177c0ee4cd95c6483eb378978fdd1cee` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |
| `Program/TT0127m_.ttf` | `e3b5229e753851acab9450fcad1acd9f89412f7bdaebfb6fbf25fc0536ab02d2` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |
| `Program/TT0085m_.ttf` | `e738818f4bbf3f29c68601fe5cb16cb045650e7d1854806e584204fd2686ed4c` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |
| `Program/TT0298m_.ttf` | `a36d52dec6c6216e2dce6f0979c715e5454a0d18647bedd03096f33dbd3d707f` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |
| `Program/TT0299m_.ttf` | `fb1ce5027aa0a0cd3817f559e63fe4d28b6e125c0c32d3635337d2acfb109519` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |
| `Program/TT0300m_.ttf` | `83a70d460edbdc1a804764d6b17de2189765a5eb18cf598e8fa7e88058d67a79` | TrueType Font data | Opened with `ret 0` in `/tmp/shandalar-mtg-program-planeswalker-font-retest-cx.log`. |

## Active Program Card Rendering Assets

These `Program/CardArt/` assets are required by `Program/Drawcardlib.dll` when
launching from `C:\Shandalar\Program`. A visible 2026-06-03 CrossOver dialog
reported missing `C:\Shandalar\Program\CARDART\ManaSymbols.pic`; a later
bounded exact-path log reached missing
`C:\Shandalar\Program\CARDART\modern\Triggering.png`; a follow-up bounded log
after that copy loaded `Triggering.png` and reached missing
`C:\Shandalar\Program\CARDART\planeswalker\LoyaltyBase.png`; a 2026-06-04
bounded log after the loyalty copy reached missing
`C:\Shandalar\Program\CARDART\modern\CardOv_Nyx.png`. The files below were
copied from the preserved `Mods/Art/_undo/.../CardArt` snapshot into both this
checkout and the local `MTG` copied install.

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Program/CardArt/ManaSymbols.pic` | `60662a25dce90dc8d4cd0b0227fe62c33b50ac95115711428d463770b8d42cbd` | PNG image data, 1508 x 26 RGBA | Required by drawcardlib `ManaSymbols` config. |
| `Program/CardArt/Expansion_Symbols.pic` | `01264f3dd6b9a8b5576b50bba49e951cb3fbdba1d33aee2b7ee8a9530d5e7348` | PNG image data, 3600 x 1875 RGBA | Required when expansion symbols are visible. |
| `Program/CardArt/Watermarks.pic` | `ec276a27c79a8cea55cdcb5474cbc5b96071f3744c18d7fc466ea6c503892c9c` | PNG image data, 750 x 600 RGBA | Required by watermark rendering config. |
| `Program/CardArt/CardCounters.png` | `8d26128c1932b22f25b84b96d8d01e9b2dce008cd96265f3efabdc0c5f11ecbb` | PNG image data, 1152 x 1440 RGBA | `Program/DuelArt/Duel.dat` sets `Cardcounters=Cardcounters.png`. |
| `Program/CardArt/Modern/Triggering.png` | `a8c94fc5b58540f884e799a1603f65dd61d99af9e50efee4762b4021bedc6f00` | PNG image data, 224 x 224 RGBA | Required by the Modern card-frame config after the first top-level drawcardlib assets load. |
| `Program/CardArt/Modern/CardOv_Nyx.png` | `b4bbf12f1f9851e2526ba25ad9b3de147fafa6aa7b1bc4400616b65aaf25209d` | PNG image data, 337 x 484 RGBA | Generic Modern Nyx overlay required because `Program/DuelArt/Duel.dat` maps the Modern `CardOv_*Nyx` frame entries to `CardOv_Nyx.png`. |
| `Program/CardArt/Planeswalker/LoyaltyBase.png` | `7413ba6227b9b07a491a2730e170525ea4744d188e31e2665abc2361ebd6e79e` | PNG image data, 62 x 39 RGBA | Required by the Planeswalker card-frame config after `Triggering.png` loads. |
| `Program/CardArt/Planeswalker/LoyaltyMinus.png` | `89f01e1bda607459ea6560c0b6608a9aab409799c05cd00279fee6d0bfd82cb9` | PNG image data, 62 x 43 RGBA | Planeswalker loyalty badge image from the preserved Program-style snapshot. |
| `Program/CardArt/Planeswalker/LoyaltyPlus.png` | `ad4b8971dd43955ccfd3daf9020b3a6f60c0a8fe9f21b73847c07a81b12af3ef` | PNG image data, 62 x 43 RGBA | Planeswalker loyalty badge image from the preserved Program-style snapshot. |
| `Program/CardArt/Planeswalker/LoyaltyZero.png` | `8faf7ec5225538bcb97b539a1614282007ea484317411806a311f1c2d800ccef` | PNG image data, 62 x 43 RGBA | Planeswalker loyalty badge image from the preserved Program-style snapshot. |

## Preserved Reference Executables

| Path | SHA-256 | File type | Why preserved |
| --- | --- | --- | --- |
| `FaceMaker-Original.exe` | `0471afcd0288a07422355ff2af224c40f8b29dc0a864eed90b3399e285f42c7e` | PE32 GUI Intel 80386 | Original FaceMaker reference for patch lineage. |
| `FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | PE32 GUI Intel 80386 | Korath/no-resolution helper; also observed as `FaceMaker-nores.exe /S` in the 2026-05-31 visible S2 new-game path. |
| `Program/FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | PE32 GUI Intel 80386 | Program-tree copy of the same no-resolution helper; preserve until all character-creation paths are understood. |

## Protected Cleanup False Positives

| Path | SHA-256 | File type | Why protected |
| --- | --- | --- | --- |
| `MENUBAK.PIC` | `34b3acb232d2a40c7e807b70cfcd3c9b95fe47a36e2061e41a6cc0d91c42b335` | data | Backup-looking name, but `.PIC` is a runtime resource family here. |
| `Program/MENUBAK.PIC` | `34b3acb232d2a40c7e807b70cfcd3c9b95fe47a36e2061e41a6cc0d91c42b335` | data | Same bytes as root. |
| `WINBAK01.PIC` | `982ac2bf1d968f3852616750f77a38e3ed246fe24ab6262c87c825fe9d2fd251` | data | Runtime-looking window backdrop/resource. |
| `Program/WINBAK01.PIC` | `982ac2bf1d968f3852616750f77a38e3ed246fe24ab6262c87c825fe9d2fd251` | data | Same bytes as root. |
| `WINBAK02.PIC` | `1a36a138907ebd13e947138961ec106efb342de99c01d09dc30ea27c0087c1fc` | data | Runtime-looking window backdrop/resource. |
| `Program/WINBAK02.PIC` | `1a36a138907ebd13e947138961ec106efb342de99c01d09dc30ea27c0087c1fc` | data | Same bytes as root. |
| `WORLBAK1.PIC` | `41757f0eeeea7f967869a1c9b0673a5ef822f634a38b2a7349c2da4c1b9d18fa` | data | Runtime-looking world backdrop/resource. |
| `Program/WORLBAK1.PIC` | `41757f0eeeea7f967869a1c9b0673a5ef822f634a38b2a7349c2da4c1b9d18fa` | data | Same bytes as root. |

## Generated or Archived Evidence Files

| Path | SHA-256 | File type | Status |
| --- | --- | --- | --- |
| `archive/generated-local/Duel.GID` | `e246ee3cf6bc430731dad14e6d1e0e84819418bd97450bb33707b7eec39c7ff6` | MS Windows help Bookmark | Archived generated help index. |
| `archive/generated-local/shandalar_dll.log` | `53e66f9ee0a7845250d3b5c19bcd4f2899dae05f5d855f529c21549d6e09e286` | CSV text | Archived runtime/debug log evidence. |
| `archive/backups/Rogues_Org_BAK.csv` | `61cc4cfe359ff156949366a3ab8b17a39adb766ab54e733ffeaa1f9cb0c169b8` | CSV text | Archived historical backup. |

## Refresh Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 Shandalar.exe Program/Shandalar.exe Magic.exe Program/Magic.exe FaceMaker.exe Program/FaceMaker.exe ManalinkEh.dll Program/ManalinkEh.dll zlib.dll Program/zlib.dll Shandalar.dll Program/Shandalar.dll CardArtLib.dll Program/CardArtLib.dll DeckDLL.dll Program/Deckdll.dll Drawcardlib.dll Program/Drawcardlib.dll libgcc_s_dw2-1.dll Program/libgcc_s_dw2-1.dll Cards.dat Program/Cards.dat DBInfo.dat Program/DBInfo.dat Rarity.dat Program/Rarity.dat Program/Manalink.ini Program/DuelArt/Modern.dat Program/DuelArt/Planeswalker.dat Program/TT0530m_.ttf Program/TT0127m_.ttf Program/TT0085m_.ttf Program/TT0298m_.ttf Program/TT0299m_.ttf Program/TT0300m_.ttf Program/CardArt/ManaSymbols.pic Program/CardArt/Expansion_Symbols.pic Program/CardArt/Watermarks.pic Program/CardArt/CardCounters.png Program/CardArt/Modern/Triggering.png Program/CardArt/Modern/CardOv_Nyx.png Program/CardArt/Planeswalker/LoyaltyBase.png Program/CardArt/Planeswalker/LoyaltyMinus.png Program/CardArt/Planeswalker/LoyaltyPlus.png Program/CardArt/Planeswalker/LoyaltyZero.png FaceMaker-Original.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe
file Shandalar.exe Program/Shandalar.exe Magic.exe Program/Magic.exe FaceMaker.exe Program/FaceMaker.exe ManalinkEh.dll Program/ManalinkEh.dll zlib.dll Program/zlib.dll Shandalar.dll Program/Shandalar.dll CardArtLib.dll Program/CardArtLib.dll DeckDLL.dll Program/Deckdll.dll Drawcardlib.dll Program/Drawcardlib.dll libgcc_s_dw2-1.dll Program/libgcc_s_dw2-1.dll Cards.dat Program/Cards.dat DBInfo.dat Program/DBInfo.dat Rarity.dat Program/Rarity.dat Program/Manalink.ini Program/DuelArt/Modern.dat Program/DuelArt/Planeswalker.dat Program/TT0530m_.ttf Program/TT0127m_.ttf Program/TT0085m_.ttf Program/TT0298m_.ttf Program/TT0299m_.ttf Program/TT0300m_.ttf Program/CardArt/ManaSymbols.pic Program/CardArt/Expansion_Symbols.pic Program/CardArt/Watermarks.pic Program/CardArt/CardCounters.png Program/CardArt/Modern/Triggering.png Program/CardArt/Modern/CardOv_Nyx.png Program/CardArt/Planeswalker/LoyaltyBase.png Program/CardArt/Planeswalker/LoyaltyMinus.png Program/CardArt/Planeswalker/LoyaltyPlus.png Program/CardArt/Planeswalker/LoyaltyZero.png FaceMaker-Original.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe
tools/verify-share-readiness.sh
```

`tools/verify-share-readiness.sh` parses this manifest and checks every listed
SHA-256 row against the current file bytes.
