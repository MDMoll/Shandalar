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
| `src/build.pl` | Helper that can edit `ManalinkEh.asm`, run `make`, and optionally copy DLLs to explicit or legacy `c:\magic2k` paths. | Use `--dry-run` first; legacy copies require `--legacy-copy-targets`. |

## Expected Outputs If Fixed

| Build path | Expected output | Current status |
| --- | --- | --- |
| `src/Makefile` | `ManalinkEh.dll` | Not built; dry run now prints a full compile/link plan, but required Windows-oriented tools are absent. |
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

Current result: after restoring `src/card_id.h` from `Program/src/card_id.h`,
the dry run prints the full `ManalinkEh.dll` compile/link plan. It still does
not prove a real build, because this machine lacks required Windows-oriented
tools such as `yasm`, `dlltool`, `objcopy`, and `windres`.

Header check:

```text
Program/src/card_id.h and src/card_id.h both hash to
22fd41e536d24d5bdbbcd5ef6b4003e420aed135066a033dc4c1d6dac24675c1
```

## Current Build Blockers

| Blocker | Evidence | Impact |
| --- | --- | --- |
| Generated/provenance of `src/card_id.h` is unresolved | `src/card_id.h` is restored as an exact copy of `Program/src/card_id.h`; no generator was found. | The dry-run blocker is mitigated, but source provenance remains unresolved. |
| Missing Windows-oriented toolchain pieces | `yasm`, `objcopy`, `dlltool`, and `windres` were not found. | DLL builds and resource builds cannot complete locally. |
| Build files use generic `gcc`/`g++`, not a discovered MinGW compiler. | `src/Makefile-common`. | macOS system compilers are not enough for Windows DLL output. |
| Historical helper scripts still know hard-coded Windows paths. | `src/build.pl` legacy copy targets and `src/deploy.bat` confirmed mode. | Use only opt-in/dry-run paths or a prepared Windows packaging copy. |

## Guidance

| Goal | Safer approach |
| --- | --- |
| Test a build | Use a branch or copy, install a MinGW/yasm/binutils toolchain, and keep outputs separate until verified. |
| Patch binaries | Read the patch script first; many modify `Magic.exe` in-place. Back up the target outside the repo. |
| Reproduce shipped binaries | Do not assume it is possible yet. Record source revision, generated headers, tools, and SHA-256 comparisons if attempted. |
