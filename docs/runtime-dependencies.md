# Runtime Dependencies

This page is based on local binary inspection, not on forum memory.

Verified commands were run from `/Users/mdmoll/Shandalar/Shandalar` with
`file` and `/usr/bin/objdump`.

## Architecture

| Binary | Architecture from `file` | Notes |
| --- | --- | --- |
| `Program/Shandalar.exe` | PE32 GUI Intel 80386 | 32-bit Windows executable. |
| `Program/Magic.exe` | PE32 GUI Intel 80386 | 32-bit Windows executable. |
| `Program/Shandalar.dll` | PE32 DLL Intel 80386 | 32-bit DLL. |
| `Program/ManalinkEh.dll` | PE32 DLL Intel 80386 | 32-bit DLL. |
| `Program/ManalinkEx.dll` | PE32 GUI DLL Intel 80386 | 32-bit DLL. |
| `Program/Deckdll.dll` | PE32 GUI DLL Intel 80386 | 32-bit DLL. |
| `Program/CardArtLib.dll` | PE32 DLL Intel 80386 | 32-bit DLL. |
| `Program/Drawcardlib.dll` | PE32 DLL Intel 80386 | 32-bit DLL. |
| `Program/zlib.dll` | PE32 DLL Intel 80386 | 32-bit DLL, byte-for-byte copy of root `zlib.dll`. |
| `Program/libgcc_s_dw2-1.dll` | PE32 DLL Intel 80386 | 32-bit DLL, byte-for-byte copy of root `libgcc_s_dw2-1.dll`. |

No inspected launch target is PE32+ or 64-bit. Prefer x86 runtime packages when
a runtime package is actually needed.

## Import Matrix

| Binary | Imported DLLs from `objdump -p` | Local adjacent DLLs | Dependency notes |
| --- | --- | --- | --- |
| `Program/Shandalar.exe` | `KERNEL32.dll`, `USER32.dll`, `GDI32.dll`, `comdlg32.dll`, `ADVAPI32.dll`, `SHELL32.dll`, `WINMM.dll`, `COMCTL32.dll`, `MSVFW32.dll`, `DrawCardLib.dll`, `DECKDLL.dll`, `CdTools.dll`, `CardArtLib.dll`, `MSVCRT.dll` | `Drawcardlib.dll`, `Deckdll.dll`, `CdTools.dll`, `CardArtLib.dll` | Uses classic Win32 UI, multimedia, Video for Windows, and MSVCRT. |
| `Program/Magic.exe` | `user32.dll`, `advapi32.dll`, `comctl32.dll`, `deckdll.dll`, `drawcardlib.dll`, `gdi32.dll`, `kernel32.dll`, `msvcrt.dll`, `msvfw32.dll`, `version.dll`, `winmm.dll`, `comdlg32.dll`, `manalinkeh.dll`, `manalinkex.dll` | `Deckdll.dll`, `Drawcardlib.dll`, `ManalinkEh.dll`, `ManalinkEx.dll` | Requires Manalink DLLs next to the executable. |
| `Program/Shandalar.dll` | `Cardartlib.dll`, `Deckdll.dll`, `Drawcardlib.dll`, `ADVAPI32.DLL`, `GDI32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `USER32.dll`, `WINMM.DLL` | `CardArtLib.dll`, `Deckdll.dll`, `Drawcardlib.dll` | Uses `MSIMG32.DLL`; Windows/Wine usually provide it. Program copy now matches root after the stale helper generation caused a visible Hornet fatal. |
| `Program/ManalinkEh.dll` | `kernel32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `USER32.dll` | none beyond system DLLs | Root `ManalinkEh.dll` imports more DLLs than the Program copy. |
| `Program/ManalinkEx.dll` | `kernel32.dll`, `user32.dll`, `advapi32.dll` | none beyond system DLLs | Loaded by `Magic.exe`; root copy has a different import table. |
| `Program/Drawcardlib.dll` | `Cardartlib.dll`, `image.dll`, `GDI32.dll`, `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `SHLWAPI.DLL`, `USER32.dll`, `libgcc_s_dw2-1.dll` | `CardArtLib.dll`, `Image.dll`, `libgcc_s_dw2-1.dll` | Card rendering helper. Program copy now matches root and requires adjacent `libgcc_s_dw2-1.dll` in the direct Program path. |

CrossOver `MTG` note: direct logged launch of
`C:\Shandalar\Program\Shandalar.exe` previously failed before gameplay because
`Program\zlib.dll` was missing and `Program\Image.dll` depends on it. This
checkout and the local copied `MTG` install now include `Program\zlib.dll`
matching root `zlib.dll`. A later visible Program-path run reached
`drawcardlib.dll` and reported missing
`C:\Shandalar\Program\CARDART\ManaSymbols.pic`; the repo and copied install now
also include the Program CardArt files listed in
[runtime-manifest.md](runtime-manifest.md). A bounded exact-path retest then
reached missing `C:\Shandalar\Program\CARDART\modern\Triggering.png`; a
follow-up bounded log after that copy loaded `Triggering.png` and reached
missing `C:\Shandalar\Program\CARDART\planeswalker\LoyaltyBase.png`; a
2026-06-04 bounded log after the loyalty copy reached missing
`C:\Shandalar\Program\CARDART\modern\CardOv_Nyx.png`. The four Planeswalker
loyalty images and the generic Modern Nyx overlay are now also present.
Follow-up bounded logs then reached missing
`C:\Shandalar\Program\Manalink.ini`,
`C:\Shandalar\Program\DuelArt\Modern.dat`,
`C:\Shandalar\Program\DuelArt\Planeswalker.dat`, and the six hardcoded
Program `TT*.ttf` drawcard font files; these adjacent Program config/font files
are now copied into the repo and local copied `MTG` install. Later visible
Program-path fatals exposed older Program card-data files too; `Program/Cards.dat`,
`Program/DBInfo.dat`, and `Program/Rarity.dat` now match root. A later visible
Hornet recurrence was traced to stale Program helper DLLs, so
`Program/Shandalar.dll`, `Program/CardArtLib.dll`, `Program/Deckdll.dll`, and
`Program/Drawcardlib.dll` now match root. The newer `Program/Drawcardlib.dll`
imports `libgcc_s_dw2-1.dll`, so `Program/libgcc_s_dw2-1.dll` now matches root
too. The latest bounded log opened the Program helper DLLs and card-data files
without the earlier fatal strings. An exact-path visible retest is still needed
before the Program Shandalar path can be treated as supported.
Root `C:\Shandalar\Shandalar.exe` remains the current copied-bottle Shandalar
path.

## Dependency Matrix Summary

| Binary | Architecture | Imported dependency families | Missing-likely dependencies from inspection | Notes |
| --- | --- | --- | --- | --- |
| `Program/Shandalar.exe` | PE32 GUI Intel 80386 | Win32 UI/system DLLs, `WINMM.dll`, `MSVFW32.dll`, `MSVCRT.dll`, local `DrawCardLib.dll`, `DECKDLL.dll`, `CdTools.dll`, `CardArtLib.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | Use x86 runtimes only if a missing-DLL dialog/log points to them. |
| `Program/Magic.exe` | PE32 GUI Intel 80386 | Win32 UI/system DLLs, `MSVFW32.dll`, `WINMM.dll`, local `deckdll.dll`, `drawcardlib.dll`, `manalinkeh.dll`, `manalinkex.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | `ManalinkEh.dll` and `ManalinkEx.dll` must be available from the working directory/load path. |
| `Program/Shandalar.dll` | PE32 DLL Intel 80386 | Local card/deck/rendering DLLs, `MSIMG32.DLL`, `WINMM.DLL`, `msvcrt.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | Keep root and Program helper generations synced with the active card-data trio. |
| `Program/Drawcardlib.dll` | PE32 DLL Intel 80386 | `Cardartlib.dll`, `image.dll`, GDI/GDI+, `MSIMG32.DLL`, `SHLWAPI.DLL`, `msvcrt.dll`, `libgcc_s_dw2-1.dll` | GDI+ support may matter in older bottles if `GDIPLUS.DLL` is absent or incomplete. | Preserve `CardArtLib.dll`, `Image.dll`, `libgcc_s_dw2-1.dll`, and top-level `Program/CardArt` rendering assets nearby. |

## Supporting DLL Imports

| DLL | Imported DLLs from this checkout | Notes |
| --- | --- | --- |
| `Program/CardArtLib.dll` | `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll` | GDI+ image helper. |
| `Program/Drawcardlib.dll` | `Cardartlib.dll`, `image.dll`, `GDI32.dll`, `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `SHLWAPI.DLL`, `USER32.dll`, `libgcc_s_dw2-1.dll` | Card rendering helper; current Program copy matches root and needs adjacent `Program/libgcc_s_dw2-1.dll`. |
| `Program/CdTools.dll` | `KERNEL32.dll`, `USER32.dll`, `ADVAPI32.dll` | CD/autoplay helper imported by `Shandalar.exe`. |
| `Program/Statwin.dll` | `KERNEL32.dll`, `USER32.dll`, `GDI32.dll`, `MSVFW32.dll`, `MSVCRT.dll` | UI/video-adjacent helper. |
| `Program/ManalinkEh.dll` | `kernel32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `USER32.dll` | Manalink extension DLL imported by `Magic.exe`; patched for Samite-family and generic activated damage-prevention prompt freezes, AI decision-time clamping, AI raw-mana speculation snapshot restore safety, Piranha Marsh/Bojuka Bog trigger targeting, generic AI player-only target selection, and AI ETB player-target trigger-mode handling. |
| `Program/ManalinkEx.dll` | `kernel32.dll`, `user32.dll`, `advapi32.dll` | Manalink extension DLL imported by `Magic.exe`. |
| `Program/Deckdll.dll` | `Drawcardlib.dll`, `image.dll`, `COMCTL32.DLL`, `GDI32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `SHLWAPI.DLL`, `USER32.dll` | Deck/card-data helper used by Shandalar and Magic paths; Program copy now matches root while preserving filename case. |
| `Program/Image.dll` | No `DLL Name:` lines were emitted by local `objdump -p`; binary strings include `zlib.dll`. | Keep nearby because it is imported by `Deckdll.dll`/`Drawcardlib.dll` and participates in the Program image/decompression path. |
| `Program/zlib.dll` | Same SHA-256 as root `zlib.dll`: `9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90`. | Adjacent support DLL for the Program image/decompression path. |
| `Program/Shandalar.dll`, `Program/CardArtLib.dll`, `Program/Deckdll.dll`, `Program/Drawcardlib.dll` | Same SHA-256 as their root helper DLL counterparts. | Synced after a visible Program-path Hornet fatal recurred from the stale Program helper generation. Preserve `Program/Deckdll.dll` filename case. |
| `Program/libgcc_s_dw2-1.dll` | Same SHA-256 as root `libgcc_s_dw2-1.dll`: `89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b`. | Adjacent GCC runtime helper imported by the current Program `Drawcardlib.dll`. |
| `Program/Manalink.ini` | Same SHA-256 as active root `Manalink.ini`: `30153fd22c76b0c0751c538938af46fbf25b1b51d5b4bb2bd9a2eead1b9c2f2b`. | `Deckdll` builds this path from the executable directory during direct Program-path launch. |
| `Program/DuelArt/Modern.dat` | Same SHA-256 as the preserved Program-style `Mods/Art/_undo/.../DuelArt/Modern.dat`: `9a2d70be70b70ef27036a47550bc0d549437df0c032a4e0237a217e4731e1aee`. | Modern frame config opened relative to Program `DuelArt` by logged direct Program-path launch. |
| `Program/DuelArt/Planeswalker.dat` | Same SHA-256 as the preserved Program-style `Mods/Art/_undo/.../DuelArt/Planeswalker.dat`: `619e0b9780ec204b9fbf6f48b2eb541c9d8a6f19a73f27d4d76d25828db7d369`. | Planeswalker frame config opened relative to Program `DuelArt` by logged direct Program-path launch. |
| `Program/TT0530m_.ttf`, `Program/TT0127m_.ttf`, `Program/TT0085m_.ttf`, `Program/TT0298m_.ttf`, `Program/TT0299m_.ttf`, `Program/TT0300m_.ttf` | TrueType font data matching existing root font bytes. | `drawcardlib/config.c` hardcodes these names and adds them from the Program base directory during direct Program-path launch. |
| Program CardArt files listed in [runtime-manifest.md](runtime-manifest.md), including `ManaSymbols.pic`, `Modern/Triggering.png`, `Modern/CardOv_Nyx.png`, and `Planeswalker/Loyalty*.png` | PNG image data copied from preserved `Mods/Art/_undo/.../CardArt` evidence after visible/logged `drawcardlib.dll` missing-asset findings. | Program card-rendering assets for direct Program-path launches. |

## What Was Not Found

In the inspected launch targets and important DLLs, no imports matched:

| Pattern | Result |
| --- | --- |
| `VCRUNTIME*.DLL` | Not found. |
| `MSVCP*.DLL` | Not found. |
| `MFC*.DLL` | Not found. |
| `DDRAW.DLL` | Not found. |
| `D3D*.DLL` | Not found. |

This does not prove the game never uses DirectX indirectly. It means the
primary launch binaries do not import those DLL names directly.

## Visual C++ Guidance

| Claim | Evidence | Guidance |
| --- | --- | --- |
| These launch targets are 32-bit. | `file` reports PE32 Intel 80386. | Install x86 redistributables into a bottle if needed. |
| The inspected launch targets import `MSVCRT.dll`. | `objdump -p` import tables. | A modern Windows or Wine/CrossOver bottle usually provides MSVCRT. |
| Old Manalink docs mention Visual C++ 2010 and VB6 runtime files. | `Manalink3/Program/ReadMe.txt:17-21`. | Treat this as historical/runtime troubleshooting guidance, not proof for `Program/Shandalar.exe` or `Program/Magic.exe`. |
| No `VCRUNTIME`, `MSVCP`, or `MFC` imports were found in the inspected launch targets. | Import scan. | Do not install random VC++ packages before recording the exact missing DLL or error. |

Do not commit redistributable installers to this repo.

## Local Files Worth Preserving

| File | Why |
| --- | --- |
| Root `Shandalar.exe`, root `Magic.exe`, root DLLs including `zlib.dll` | Current copied CrossOver `MTG` launch path and adjacent dependencies. |
| `Program/Shandalar.dll`, `Program/Deckdll.dll`, `Program/Drawcardlib.dll`, `Program/CardArtLib.dll`, `Program/CdTools.dll`, `Program/zlib.dll`, `Program/libgcc_s_dw2-1.dll` | Direct imports or adjacent support DLLs for `Program/Shandalar.exe` and its rendering helpers. |
| `Program/Manalink.ini`, Program `DuelArt` configs, and Program `TT*.ttf` font files | Adjacent config/font files required by logged direct Program-path runtime evidence. |
| Program CardArt files listed in [runtime-manifest.md](runtime-manifest.md) | Drawcardlib/card-rendering assets required by direct Program-path launch evidence. |
| Root and `Program/ManalinkEh.dll`, `Program/ManalinkEx.dll` | Direct imports of root/Program `Magic.exe`. Preserve the patched `ManalinkEh.dll` copies; root and `Program/` have different damage-prevention, AI clamp, AI raw-mana snapshot, Piranha Marsh trigger-target, Bojuka Bog trigger-target, generic AI player-target selector, and AI ETB trigger-mode patch offsets and hashes. |
| `Program/Shandalar.dll` | Loaded by strings evidence in `Shandalar.exe`; stale Program helper bytes caused a visible Hornet fatal even after the Program card-data trio was synced. |
| `Program/Cards.dat`, `Program/DBInfo.dat`, `Program/Rarity.dat`, `Program/Text.res`, art/sound/deck folders | Runtime data and assets referenced by strings and folder layout. |
