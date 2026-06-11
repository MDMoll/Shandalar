# Build Health

This audit started with dry-run probes only. Later controlled passes rebuilt
and deployed `DeckDLL.dll`, `CardArtLib.dll`, and `Drawcardlib.dll`. The
top-level `src` and `Program/src` trees now also produce source-local
`ManalinkEh.dll` builds with the default warnings-visible build lane, but those
outputs were not accepted as shipped runtime replacements.

Generated evidence:
[generated/code-audit/tool-availability.txt](generated/code-audit/tool-availability.txt),
[generated/code-audit/build-targets.tsv](generated/code-audit/build-targets.tsv),
and
[generated/code-audit/build-probe-results.tsv](generated/code-audit/build-probe-results.tsv).
Source snapshot evidence:
[generated/code-audit/source-snapshot-parity.tsv](generated/code-audit/source-snapshot-parity.tsv)
and
[generated/code-audit/source-snapshot-parity-summary.txt](generated/code-audit/source-snapshot-parity-summary.txt).

Read-only preflights:
`tools/check-build-prereqs.sh --report-only` and
`tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only`.

## Tool Availability

| Tool | Result | Status |
| --- | --- | --- |
| `make` | `/usr/bin/make` | verified |
| `gcc`, `g++`, `clang`, `clang++` | present | verified |
| `perl`, `python3` | present | verified |
| `yasm`, `i686-w64-mingw32-dlltool`, `i686-w64-mingw32-objcopy`, `i686-w64-mingw32-windres` | present under `/opt/homebrew/bin` and `/opt/homebrew/Cellar/mingw-w64/14.0.0/toolchain-i686` | verified for DeckDLL, CardArtLib, Drawcardlib, and top-level Manalink source builds |
| `nasm` | absent | verified |
| `cppcheck`, `scan-build`, `clang-tidy`, `infer`, `semgrep` | absent | verified |

## Build Probe Results

| Build Target | Command | Required Tools | Result | Missing Inputs | Output Expected | Risk |
| --- | --- | --- | --- | --- | --- | --- |
| `src/Makefile` | `make -C src clean && make -B -k -C src` | GNU make, MinGW gcc/g++ auto-detected from `PATH`, yasm, dlltool, objcopy | default warnings-visible build links source-local `ManalinkEh.dll` with 0 errors and 0 failed targets | output not accepted as a shipped-runtime replacement; `WARNINGS_AS_ERRORS=1` still exposes 552 warning-errors across 100 targets | `ManalinkEh.dll` | medium; output is a source-local DLL, not deployed |
| `Program/src/Makefile` | `make -C Program/src clean && make -B -k -C Program/src` | GNU make, MinGW gcc/g++ auto-detected from `PATH`, yasm, dlltool, objcopy | default warnings-visible build links source-local `ManalinkEh.dll` with 0 errors and 0 failed targets | output not accepted as a shipped-runtime replacement; `WARNINGS_AS_ERRORS=1` still exposes 538 warning-errors across 94 targets | `ManalinkEh.dll` | medium; output is a source-local DLL, not deployed |
| `src/deck/Makefile` | `(cd src/deck && make CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ DLLTOOL=i686-w64-mingw32-dlltool OBJCOPY=i686-w64-mingw32-objcopy WINDRES=i686-w64-mingw32-windres)` | Homebrew MinGW/yasm plus local import libs and legacy-safe startup linker flags | built and deployed | none for this target after compatibility fixes | `DeckDll.dll` | medium; output is a runtime DLL |
| `src/drawcardlib/Makefile` | `make -C src/drawcardlib clean && make -B -C src/drawcardlib` | Homebrew MinGW/yasm plus local import libs and legacy-safe startup linker flags | default source-local build passes; earlier controlled build was deployed | none for this target after tool auto-detection and PE `DllCharacteristics` `0x0000` verification | `Drawcardlib.dll` | medium; output is a DLL; do not deploy without runtime acceptance |
| `Program/src/drawcardlib/Makefile` | `make -B -C Program/src/drawcardlib WARNINGS_AS_ERRORS=1` | Homebrew MinGW/yasm plus local import libs and legacy-safe startup linker flags | strict source-local build passes after Program snapshot warning fixes | output not accepted as a shipped-runtime replacement | `Drawcardlib.dll` | medium; output is a DLL; do not deploy without runtime acceptance |
| `src/cardartlib/Makefile` | `(cd src/cardartlib && make CXX=i686-w64-mingw32-g++ CC=i686-w64-mingw32-gcc OBJCOPY=i686-w64-mingw32-objcopy DLLTOOL=i686-w64-mingw32-dlltool)` | Homebrew MinGW C++ plus GDI+/GDI32 | built and deployed | none for this target after Boost removal and PE `DllCharacteristics` `0x0000` verification | `CardArtLib.dll` | medium; output is a runtime DLL |
| `Program/src/cardartlib/Makefile` | `make -B -C Program/src/cardartlib WARNINGS_AS_ERRORS=1` | Homebrew MinGW C++ plus GDI+/GDI32 | strict source-local build passes | output not accepted as a shipped-runtime replacement | `CardArtLib.dll` | medium; output is a DLL; do not deploy without runtime acceptance |
| `src/manalinkex/Makefile` | `make -B -C src/manalinkex` | Homebrew MinGW C plus matching `objcopy` auto-detected from `PATH` | source-local build passes | output not accepted as a shipped-runtime replacement | `ManalinkEx.dll` | medium; output is a DLL; do not deploy without runtime acceptance |
| `src/patches/Makefile` and `Program/src/patches/Makefile` | `make -B -C src/patches` and `make -B -C Program/src/patches` | Homebrew MinGW C auto-detected from `PATH` | patch helper EXEs build without running or installing them | patch helpers remain potentially mutating if executed later | patch EXEs | medium; build-only is non-mutating, execution is high risk |

## Build Conclusions

| Question | Answer |
| --- | --- |
| Can `src/` rebuild shipped binaries today? | partially; `src/deck/DeckDll.dll`, `src/cardartlib/CardArtLib.dll`, and `src/drawcardlib/Drawcardlib.dll` rebuilt and were deployed in earlier controlled passes, and top-level `src` now links a source-local `ManalinkEh.dll`. That top-level output has not been accepted as a shipped-runtime replacement. |
| Can `Program/src/` rebuild shipped binaries today? | partially; top-level `Program/src` now links a source-local `ManalinkEh.dll`, but no Program-source output has been accepted as a shipped-runtime replacement. |
| Are shipped binaries reproducible from source? | not tested and currently unsupported by evidence. |
| Are outputs separated from checked-in runtime binaries? | not consistently; historical helpers can copy into runtime-style paths. |
| Is a native Windows/MinGW environment likely needed? | not for the build-only targets verified on this machine; Homebrew MinGW now covers top-level source builds, helper DLLs, and patch helper EXEs. Runtime acceptance and binary replacement remain separate decisions. |

## Blockers

| Blocker | Evidence | Fix Direction |
| --- | --- | --- |
| Generated card id header process | mitigated; `src/card_id.h` and `Program/src/card_id.h` are exact matches (`d9f91726070a7246602e0e6b3b1121d3db0681c3c1efb91d95ed6e365f2896e8`) and their repaired tail follows the active root `magic_updater/Manalink.csv` namespace. | Keep the mirrored header until a checked-in generator/provenance path is documented. |
| Unproven Windows DLL build coverage for shipped replacement | Homebrew MinGW/yasm/binutils are available and were used for top-level source builds, DeckDLL, CardArtLib, Drawcardlib, ManalinkEx, and patch helper EXEs; those source-local outputs were not accepted as shipped-runtime replacements. | Keep target-specific build logs and hashes before replacing any additional runtime binaries. |
| Divergent source snapshots | `tools/check-source-snapshot-parity.sh --report-only` records 77 matching paths, 145 differing paths, and 100 paths missing from `Program/src`; current exact-match files and safety markers pass. | Keep the parity guard, then establish source provenance before claiming fixes affect shipped runtime. |
| Mutating helper scripts | `src/build.pl` can rewrite `ManalinkEh.asm` when intentionally invoked with a card name; `src/deploy.bat` confirmed mode copies/deletes under `c:\magic2k`. | Use `--dry-run` or explicit confirmation only in controlled build/packaging copies. |
| Strict source warning debt | `make -B -k -C src WARNINGS_AS_ERRORS=1` and `make -B -k -C Program/src WARNINGS_AS_ERRORS=1` still fail on historical warnings such as packed-member addresses, fallthroughs, strict-aliasing, array-bounds, and attribute redeclarations; `Program/src/drawcardlib WARNINGS_AS_ERRORS=1` also fails on older Program snapshot warnings. | Keep default builds warnings-visible; use `WARNINGS_AS_ERRORS=1` for focused cleanup targets rather than treating the whole historical tree as strict-clean. |

## Fixes Applied After Audit

| Fix | Evidence | Remaining Limit |
| --- | --- | --- |
| Added `tools/check-build-prereqs.sh`. | `tools/check-build-prereqs.sh --report-only` reports the restored matching `src/card_id.h`, local MinGW/yasm/binutils availability, and the source provenance warning without building or copying. | It is a preflight, not a full build repair or runtime replacement proof. |
| Hardened `src/build.pl`. | `perl -c src/build.pl` and `perl src/build.pl --help`; legacy `c:\magic2k` copies now require `--legacy-copy-targets` or explicit `--copy-to`. | The script can still edit `ManalinkEh.asm` when intentionally run with a card name and without `--dry-run`. Use in a disposable/build branch. |
| Mirrored `card_id.h` to the active card-data namespace. | `shasum -a 256 Program/src/card_id.h src/card_id.h`; focused Oath/rules-engine object builds pass with MinGW. | The active source is root `magic_updater/Manalink.csv`; a checked-in generator was not found. |
| Made top-level source manifests explicit. | `make -C src -n` no longer includes root-only snapshot fragments that are absent from `Program/src` and duplicate or contradict active set-based implementations. | This chooses the active build set; it does not delete or reinterpret historical source fragments. |
| Added explicit warnings-as-errors control. | `make -B -k -C src` and `make -B -k -C Program/src` link source-local `ManalinkEh.dll`; `make -B -C src WARNINGS_AS_ERRORS=1 functions/functions.o` and the matching Program command still pass strictly. Drawcardlib makefiles use the same switch. | Full-tree strict builds remain warning-debt audits. |
| Fixed Program snapshot header storage. | `Program/src/manalink.h` now declares `card_coded` and `set_legacy_effect_name_addr` as `extern`, with storage in `Program/src/functions/functions.c`; Program top-level link now succeeds in the default build lane. | This is a modern MinGW compatibility fix; it does not prove binary equivalence with historical builds. |
| Added source snapshot parity reporting. | `tools/check-source-snapshot-parity.sh` records full snapshot divergence as evidence while enforcing exact-match files and mirrored safety markers for current source fixes. | It guards against accidental drift in the current safety pass; it does not choose the authoritative source snapshot or prove rebuild reproducibility. |
| Guarded draft output file handling. | `src/cards/draft.c` and `Program/src/cards/draft.c` use `open_draft_output()` for runtime-local draft logs/deck writes and remain exact matches. | Output paths still depend on the runtime working directory; relocation needs a draft-mode copy test. |
