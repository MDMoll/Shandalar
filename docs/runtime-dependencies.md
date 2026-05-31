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

No inspected launch target is PE32+ or 64-bit. Prefer x86 runtime packages when
a runtime package is actually needed.

## Import Matrix

| Binary | Imported DLLs from `objdump -p` | Local adjacent DLLs | Dependency notes |
| --- | --- | --- | --- |
| `Program/Shandalar.exe` | `KERNEL32.dll`, `USER32.dll`, `GDI32.dll`, `comdlg32.dll`, `ADVAPI32.dll`, `SHELL32.dll`, `WINMM.dll`, `COMCTL32.dll`, `MSVFW32.dll`, `DrawCardLib.dll`, `DECKDLL.dll`, `CdTools.dll`, `CardArtLib.dll`, `MSVCRT.dll` | `Drawcardlib.dll`, `Deckdll.dll`, `CdTools.dll`, `CardArtLib.dll` | Uses classic Win32 UI, multimedia, Video for Windows, and MSVCRT. |
| `Program/Magic.exe` | `user32.dll`, `advapi32.dll`, `comctl32.dll`, `deckdll.dll`, `drawcardlib.dll`, `gdi32.dll`, `kernel32.dll`, `msvcrt.dll`, `msvfw32.dll`, `version.dll`, `winmm.dll`, `comdlg32.dll`, `manalinkeh.dll`, `manalinkex.dll` | `Deckdll.dll`, `Drawcardlib.dll`, `ManalinkEh.dll`, `ManalinkEx.dll` | Requires Manalink DLLs next to the executable. |
| `Program/Shandalar.dll` | `Cardartlib.dll`, `Deckdll.dll`, `Drawcardlib.dll`, `GDI32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `USER32.dll`, `WINMM.DLL` | `CardArtLib.dll`, `Deckdll.dll`, `Drawcardlib.dll` | Uses `MSIMG32.DLL`; Windows/Wine usually provide it. |
| `Program/ManalinkEh.dll` | `kernel32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `USER32.dll` | none beyond system DLLs | Root `ManalinkEh.dll` imports more DLLs than the Program copy. |
| `Program/ManalinkEx.dll` | `kernel32.dll`, `user32.dll`, `advapi32.dll` | none beyond system DLLs | Loaded by `Magic.exe`; root copy has a different import table. |
| `Program/Drawcardlib.dll` | `Cardartlib.dll`, `image.dll`, `GDI32.dll`, `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `SHLWAPI.DLL`, `USER32.dll` | `CardArtLib.dll`, `Image.dll` | Card rendering helper. Root copy showed a different import set. |

CrossOver `MTG` note: direct logged launch of
`C:\Shandalar\Program\Shandalar.exe` fails before gameplay because
`Program\zlib.dll` is missing and `Program\Image.dll` depends on it. Root
`C:\Shandalar\Shandalar.exe` is the current copied-bottle Shandalar path because
root `zlib.dll` and the root DLL set are present.

## Dependency Matrix Summary

| Binary | Architecture | Imported dependency families | Missing-likely dependencies from inspection | Notes |
| --- | --- | --- | --- | --- |
| `Program/Shandalar.exe` | PE32 GUI Intel 80386 | Win32 UI/system DLLs, `WINMM.dll`, `MSVFW32.dll`, `MSVCRT.dll`, local `DrawCardLib.dll`, `DECKDLL.dll`, `CdTools.dll`, `CardArtLib.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | Use x86 runtimes only if a missing-DLL dialog/log points to them. |
| `Program/Magic.exe` | PE32 GUI Intel 80386 | Win32 UI/system DLLs, `MSVFW32.dll`, `WINMM.dll`, local `deckdll.dll`, `drawcardlib.dll`, `manalinkeh.dll`, `manalinkex.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | `ManalinkEh.dll` and `ManalinkEx.dll` must be available from the working directory/load path. |
| `Program/Shandalar.dll` | PE32 DLL Intel 80386 | Local card/deck/rendering DLLs, `MSIMG32.DLL`, `WINMM.DLL`, `msvcrt.dll` | No direct `VCRUNTIME`, `MSVCP`, `MFC`, `DDRAW`, or `D3D` imports found. | Likely part of patched Shandalar runtime behavior. |
| `Program/Drawcardlib.dll` | PE32 DLL Intel 80386 | `Cardartlib.dll`, `image.dll`, GDI/GDI+, `MSIMG32.DLL`, `SHLWAPI.DLL`, `msvcrt.dll` | GDI+ support may matter in older bottles if `GDIPLUS.DLL` is absent or incomplete. | Preserve `CardArtLib.dll` and `Image.dll` nearby. |

## Supporting DLL Imports

| DLL | Imported DLLs from this checkout | Notes |
| --- | --- | --- |
| `Program/CardArtLib.dll` | `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll` | GDI+ image helper. |
| `Program/Drawcardlib.dll` | `Cardartlib.dll`, `image.dll`, `GDI32.dll`, `GDIPLUS.DLL`, `KERNEL32.dll`, `msvcrt.dll`, `MSIMG32.DLL`, `SHLWAPI.DLL`, `USER32.dll` | Card rendering helper. |
| `Program/CdTools.dll` | `KERNEL32.dll`, `USER32.dll`, `ADVAPI32.dll` | CD/autoplay helper imported by `Shandalar.exe`. |
| `Program/Statwin.dll` | `KERNEL32.dll`, `USER32.dll`, `GDI32.dll`, `MSVFW32.dll`, `MSVCRT.dll` | UI/video-adjacent helper. |
| `Program/ManalinkEh.dll` | `kernel32.dll`, `KERNEL32.dll`, `msvcrt.dll`, `USER32.dll` | Manalink extension DLL imported by `Magic.exe`. |
| `Program/ManalinkEx.dll` | `kernel32.dll`, `user32.dll`, `advapi32.dll` | Manalink extension DLL imported by `Magic.exe`. |
| `Program/Deckdll.dll`, `Program/Image.dll` | No `DLL Name:` lines were emitted by local `objdump -p`. | Keep nearby anyway because they are direct imports or supporting DLLs. |

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
| `Program/Deckdll.dll`, `Program/Drawcardlib.dll`, `Program/CardArtLib.dll`, `Program/CdTools.dll` | Direct imports of `Program/Shandalar.exe`. |
| `Program/ManalinkEh.dll`, `Program/ManalinkEx.dll` | Direct imports of `Program/Magic.exe`. |
| `Program/Shandalar.dll` | Loaded by strings evidence in `Shandalar.exe`; likely part of patched Shandalar behavior. |
| `Program/Cards.dat`, `Program/DBInfo.dat`, `Program/Rarity.dat`, `Program/Text.res`, art/sound/deck folders | Runtime data and assets referenced by strings and folder layout. |
