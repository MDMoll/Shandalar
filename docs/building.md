# Building

This repo contains build files and source. The DeckDLL, CardArtLib,
Drawcardlib, and top-level Manalink source paths have current local rebuild
proof, but that still does not prove an end-to-end rebuild of the shipped game.

## Build Surface

| Path | Appears to build | Evidence |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | `OUTPUT = ManalinkEh.dll`; includes `Makefile-common` and `Makefile-targets`. |
| `src/deck/Makefile` | `DeckDll.dll` | `OUTPUT = DeckDll.dll`; uses `windres` and local `Drawcardlib`/`image` import libs. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | `TGTDLL = Drawcardlib.dll`; auto-detects prefixed MinGW `gcc`/`dlltool`/`objcopy` when available and uses `yasm`. |
| `src/cardartlib/Makefile` | `CardArtLib.dll` | `OUTPUT = CardArtLib.dll`; uses Win32 directory enumeration plus GDI+/GDI32. |
| `src/patches/*` | In-place binary patches | Many scripts state they update `Magic.exe`, `Shandalar.exe`, or DLLs in-place. |
| `src/build.pl` | Helper that can edit `ManalinkEh.asm`, run `make`, and optionally copy DLLs to explicit or legacy `c:\magic2k` paths. | Use `--dry-run` first; legacy copies require `--legacy-copy-targets`. |

## Expected Outputs If Fixed

| Build path | Expected output | Current status |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | Default warnings-visible build passes locally with Homebrew MinGW/yasm/binutils and links a source-local DLL. It is not accepted as a runtime replacement. |
| `src/deck/Makefile` | `DeckDll.dll` | Built locally with Homebrew MinGW/yasm overrides plus legacy-safe startup flags, then deployed to root plus `Program/`; SHA-256 `5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0`. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | Default source-local build passes with Homebrew MinGW/yasm auto-detection, GDI+ external-codec suppression, notification-hook status checks, checked GDI+ wrappers for high-frequency render operations, best-effort interpolation setup for Wine/CrossOver chooser HDCs, and legacy-safe PE startup linker flags (`___ImageBase=__image_base__`, `--disable-dynamicbase`, `--disable-nxcompat`). The accepted runtime DLL was earlier deployed to root plus `Program/`; SHA-256 `7b585c3f2c57bdbda66ceaeecdd48a8e97d67bb32010c1455165fcb44cd2966c`. |
| `src/cardartlib/Makefile` | `CardArtLib.dll` | Built locally with Homebrew MinGW C++ overrides after removing the old Boost filesystem dependency, then deployed to root plus `Program/`; SHA-256 `c1a68591059ff3e650104bf711d4e3f0c9a01a232db2e594af64aaa6846b3c1d`. The accepted build keeps PE `DllCharacteristics` `0x0000` and links with `___ImageBase=__image_base__`. |
| `src/patches/*` | Patched binaries/DLLs in-place | Not run; patch scripts are potentially mutating. |

## Local Tool Check

| Tool | Result |
| --- | --- |
| `make` | Found at `/usr/bin/make`. |
| `perl` | Found at `/usr/bin/perl`. |
| `g++` | Found at `/usr/bin/g++`. |
| `i686-w64-mingw32-gcc` | Found at `/opt/homebrew/bin/i686-w64-mingw32-gcc`. |
| `i686-w64-mingw32-g++` | Found at `/opt/homebrew/bin/i686-w64-mingw32-g++`. |
| `i386-mingw32-gcc` | Not found. |
| `yasm` | Found at `/opt/homebrew/bin/yasm`. |
| `i686-w64-mingw32-objcopy` | Found at `/opt/homebrew/bin/i686-w64-mingw32-objcopy`. |
| `i686-w64-mingw32-dlltool` | Found at `/opt/homebrew/bin/i686-w64-mingw32-dlltool`. |
| `i686-w64-mingw32-windres` | Found at `/opt/homebrew/bin/i686-w64-mingw32-windres`. |

## Tried

Command:

```sh
cd /Users/mdmoll/Shandalar/Shandalar/src
make -n
```

Current result: after mirroring `src/card_id.h` and `Program/src/card_id.h`
from the active root `magic_updater/Manalink.csv` namespace and excluding
root-only snapshot fragments from the active wildcard manifest,
`make -B -k -C src` and `make -B -k -C Program/src` both link source-local
`ManalinkEh.dll` outputs with 0 errors and 0 failed targets. These outputs have
not been accepted as runtime replacements. CardArtLib and Drawcardlib must keep
legacy-safe PE startup linker flags; a 2026-06-08 CrossOver root launch review
showed the first GDI+ rebuild crashed during `DrawCardLib.dll` process attach
when it carried `DYNAMIC_BASE`/`NX_COMPAT` characteristics.

Header check:

```text
Program/src/card_id.h and src/card_id.h both hash to
d9f91726070a7246602e0e6b3b1121d3db0681c3c1efb91d95ed6e365f2896e8
```

## Current Build Blockers

| Blocker | Evidence | Impact |
| --- | --- | --- |
| Generated `card_id.h` process is still ad hoc | `src/card_id.h` and `Program/src/card_id.h` are exact matches; the current tail was regenerated from root `magic_updater/Manalink.csv`, which matches the active runtime card-data namespace. No checked-in generator was found. | The dry-run blocker is mitigated, but future card-data updates need a documented generator or repeatable script. |
| Windows-oriented build coverage is still incomplete for most targets | Homebrew MinGW/yasm/binutils are present and were used for `src/deck`, `src/cardartlib`, and `src/drawcardlib`, but the other DLL targets have not been built with recorded, accepted runtime outputs. | Do not infer full-runtime rebuild support from target-specific DeckDLL/CardArtLib/Drawcardlib proofs or a dry-run plan. |
| Historical helper scripts still know hard-coded Windows paths. | `src/build.pl` legacy copy targets and `src/deploy.bat` confirmed mode. | Use only opt-in/dry-run paths or a prepared Windows packaging copy. |
| Full-tree strict warnings are not clean. | `make -B -k -C src WARNINGS_AS_ERRORS=1` reports 552 warning-errors; `make -B -k -C Program/src WARNINGS_AS_ERRORS=1` reports 538. Focused `functions/functions.o` strict builds pass on both snapshots. Program Drawcardlib also has strict warning debt, though its default build passes. | Treat strict full builds as warning-debt audits, not the default artifact build path. |

## Toolchain Defaults

`src/Makefile-common` and `Program/src/Makefile-common` auto-detect a prefixed
MinGW compiler when `i686-w64-mingw32-gcc` or `i386-mingw32-gcc` is on `PATH`.
If neither prefixed compiler is present, they fall back to the historical bare
`gcc`/`g++` tool names for older MinGW shells. Command-line tool overrides still
take precedence.

The top-level `src`, `Program/src`, and Drawcardlib makefiles default to a
warnings-visible build lane so historical warnings do not prevent artifact
creation. Pass `WARNINGS_AS_ERRORS=1` when using those makefiles as a strict
warning cleanup gate.

## Guidance

| Goal | Safer approach |
| --- | --- |
| Test a build | Use a branch or copy, install a MinGW/yasm/binutils toolchain, and keep outputs separate until verified. |
| Patch binaries | Read the patch script first; many modify `Magic.exe` in-place. Back up the target outside the repo. |
| Reproduce shipped binaries | Do not assume it is possible yet. Record source revision, generated headers, tools, and SHA-256 comparisons if attempted. |
