# Building

This repo contains build files and source, but this pass did not prove an
end-to-end rebuild of the shipped game.

## Build Surface

| Path | Appears to build | Evidence |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | `OUTPUT = ManalinkEh.dll`; includes `Makefile-common` and `Makefile-targets`. |
| `src/deck/Makefile` | `DeckDll.dll` | `OUTPUT = DeckDll.dll`; uses `windres` and local `Drawcardlib`/`image` import libs. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | `TGTDLL = Drawcardlib.dll`; uses `gcc -m32`, `yasm`, `dlltool`, `objcopy`. |
| `src/cardartlib/Makefile` | `CardArtLib.dll` | `OUTPUT = CardArtLib.dll`; links Boost filesystem/system libraries. |
| `src/patches/*` | In-place binary patches | Many scripts state they update `Magic.exe`, `Shandalar.exe`, or DLLs in-place. |
| `src/build.pl` | Helper that edits `ManalinkEh.asm`, runs `make`, then copies DLLs to hard-coded `c:\magic2k` paths. | Lines 47-53. |

## Expected Outputs If Fixed

| Build path | Expected output | Current status |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | Not built; dry run stops before a complete build graph. |
| `src/deck/Makefile` | `DeckDll.dll` | Not attempted beyond inspection. |
| `src/drawcardlib/Makefile` | `Drawcardlib.dll` | Not attempted beyond inspection. |
| `src/cardartlib/Makefile` | `CardArtLib.dll` | Not attempted beyond inspection. |
| `src/patches/*` | Patched binaries/DLLs in-place | Not run; patch scripts are potentially mutating. |

## Local Tool Check

| Tool | Result |
| --- | --- |
| `make` | Found at `/usr/bin/make`. |
| `perl` | Found at `/usr/bin/perl`. |
| `g++` | Found at `/usr/bin/g++`. |
| `i686-w64-mingw32-gcc` | Not found. |
| `i386-mingw32-gcc` | Not found. |
| `yasm` | Not found. |
| `objcopy` | Not found. |
| `dlltool` | Not found. |
| `windres` | Not found. |

## Tried

Command:

```sh
cd /Users/mdmoll/Shandalar/Shandalar/src
make -n
```

Result: dry run printed many `gcc` compile commands, then stopped with:

```text
make: *** No rule to make target `functions/utility.obj', needed by `ManalinkEh.dll'.  Stop.
```

Additional file check:

```text
src/functions/utility.cpp exists
src/card_id.h does not exist
```

Because `Makefile-targets` lists `$(BASEDIR)/card_id.h` as a prerequisite for
`.cpp` object files, the missing generated/header file likely contributes to
the dry-run failure.

## Current Build Blockers

| Blocker | Evidence | Impact |
| --- | --- | --- |
| Missing `src/card_id.h` | `ls` reported no such file. | C++ object rules cannot resolve all prerequisites. |
| Missing Windows-oriented toolchain pieces | `yasm`, `objcopy`, `dlltool`, and `windres` were not found. | DLL builds and resource builds cannot complete locally. |
| Build files use generic `gcc`/`g++`, not a discovered MinGW compiler. | `src/Makefile-common`. | macOS system compilers are not enough for Windows DLL output. |
| Helper scripts copy to hard-coded Windows paths. | `src/build.pl:51-53`, `src/deploy.bat:1-44`. | Running them unmodified would not be safe or portable. |

## Guidance

| Goal | Safer approach |
| --- | --- |
| Test a build | Use a branch or copy, install a MinGW/yasm/binutils toolchain, and keep outputs separate until verified. |
| Patch binaries | Read the patch script first; many modify `Magic.exe` in-place. Back up the target outside the repo. |
| Reproduce shipped binaries | Do not assume it is possible yet. Record source revision, generated headers, tools, and SHA-256 comparisons if attempted. |
