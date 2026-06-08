# Building

This repo contains build files and source, but only the DeckDLL, CardArtLib,
and Drawcardlib paths have current local rebuild proof. This still does not prove
an end-to-end rebuild of the shipped game.

## Build Surface

| Path | Appears to build | Evidence |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | `OUTPUT = ManalinkEh.dll`; includes `Makefile-common` and `Makefile-targets`. |
| `src/deck/Makefile` | `DeckDll.dll` | `OUTPUT = DeckDll.dll`; uses `windres` and local `Drawcardlib`/`image` import libs. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | `TGTDLL = Drawcardlib.dll`; uses `gcc -m32`, `yasm`, `dlltool`, `objcopy`. |
| `src/cardartlib/Makefile` | `CardArtLib.dll` | `OUTPUT = CardArtLib.dll`; uses Win32 directory enumeration plus GDI+/GDI32. |
| `src/patches/*` | In-place binary patches | Many scripts state they update `Magic.exe`, `Shandalar.exe`, or DLLs in-place. |
| `src/build.pl` | Helper that can edit `ManalinkEh.asm`, run `make`, and optionally copy DLLs to explicit or legacy `c:\magic2k` paths. | Use `--dry-run` first; legacy copies require `--legacy-copy-targets`. |

## Expected Outputs If Fixed

| Build path | Expected output | Current status |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | Not accepted as a runtime replacement; dry run now prints a full compile/link plan, and the local MinGW/yasm/binutils tools are present. |
| `src/deck/Makefile` | `DeckDll.dll` | Built locally with Homebrew MinGW/yasm overrides plus legacy-safe startup flags, then deployed to root plus `Program/`; SHA-256 `5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0`. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | Built locally with Homebrew MinGW/yasm path overrides, modern compiler compatibility flags (`-fcommon -Wno-error`), GDI+ external-codec suppression, notification-hook status checks, checked GDI+ wrappers for high-frequency render operations, and legacy-safe PE startup linker flags (`___ImageBase=__image_base__`, `--disable-dynamicbase`, `--disable-nxcompat`), then deployed to root plus `Program/`; SHA-256 `24fbf84f26dd8258cd9ae23350971080fa2f192a210f610ca6a135b669b6ead9`. |
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

Current result: after restoring `src/card_id.h` from `Program/src/card_id.h`,
the dry run prints the full `ManalinkEh.dll` compile/link plan. It still does
not prove a real Manalink build; only `src/deck/DeckDll.dll`,
`src/cardartlib/CardArtLib.dll`, and `src/drawcardlib/Drawcardlib.dll` have
been rebuilt with explicit `i686-w64-mingw32-*` tool overrides. CardArtLib and
Drawcardlib must keep legacy-safe PE startup linker flags; a 2026-06-08
CrossOver root launch review showed the first GDI+ rebuild crashed during
`DrawCardLib.dll` process attach when it carried `DYNAMIC_BASE`/`NX_COMPAT`
characteristics.

Header check:

```text
Program/src/card_id.h and src/card_id.h both hash to
22fd41e536d24d5bdbbcd5ef6b4003e420aed135066a033dc4c1d6dac24675c1
```

## Current Build Blockers

| Blocker | Evidence | Impact |
| --- | --- | --- |
| Generated/provenance of `src/card_id.h` is unresolved | `src/card_id.h` is restored as an exact copy of `Program/src/card_id.h`; no generator was found. | The dry-run blocker is mitigated, but source provenance remains unresolved. |
| Missing or unproven Windows-oriented toolchain pieces for most targets | Homebrew MinGW/yasm/binutils are present and were used for `src/deck`, `src/cardartlib`, and `src/drawcardlib`, but the other DLL targets have not been built with recorded, accepted runtime outputs. | Do not infer full-runtime rebuild support from target-specific DeckDLL/CardArtLib/Drawcardlib proofs or a dry-run plan. |
| Build files use generic `gcc`/`g++`, not a discovered MinGW compiler. | `src/Makefile-common`. | macOS system compilers are not enough for Windows DLL output. |
| Historical helper scripts still know hard-coded Windows paths. | `src/build.pl` legacy copy targets and `src/deploy.bat` confirmed mode. | Use only opt-in/dry-run paths or a prepared Windows packaging copy. |

## Guidance

| Goal | Safer approach |
| --- | --- |
| Test a build | Use a branch or copy, install a MinGW/yasm/binutils toolchain, and keep outputs separate until verified. |
| Patch binaries | Read the patch script first; many modify `Magic.exe` in-place. Back up the target outside the repo. |
| Reproduce shipped binaries | Do not assume it is possible yet. Record source revision, generated headers, tools, and SHA-256 comparisons if attempted. |
