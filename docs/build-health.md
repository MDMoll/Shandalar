# Build Health

This audit used dry-run probes only. No shipped binaries were rebuilt or
overwritten.

Generated evidence:
[generated/code-audit/tool-availability.txt](generated/code-audit/tool-availability.txt),
[generated/code-audit/build-targets.tsv](generated/code-audit/build-targets.tsv),
and
[generated/code-audit/build-probe-results.tsv](generated/code-audit/build-probe-results.tsv).

## Tool Availability

| Tool | Result | Status |
| --- | --- | --- |
| `make` | `/usr/bin/make` | verified |
| `gcc`, `g++`, `clang`, `clang++` | present | verified |
| `perl`, `python3` | present | verified |
| `yasm`, `dlltool`, `objcopy`, `windres`, `nasm` | absent | verified |
| `cppcheck`, `scan-build`, `clang-tidy`, `infer`, `semgrep` | absent | verified |

## Build Probe Results

| Build Target | Command | Required Tools | Result | Missing Inputs | Output Expected | Risk |
| --- | --- | --- | --- | --- | --- | --- |
| `src/Makefile` | `(cd src && make -n)` | GNU make, gcc/g++, yasm, dlltool, objcopy | fails dry-run | `src/card_id.h`; object rule for `functions/utility.obj` | `ManalinkEh.dll` | low for dry-run; high for real build |
| `Program/src/Makefile` | `(cd Program/src && make -n)` | GNU make, gcc/g++, yasm, dlltool, objcopy | dry-run prints build plan | Windows DLL toolchain absent for real build | `ManalinkEh.dll` | low for dry-run; high for real build |
| `src/deck/Makefile` | `(cd src/deck && make -n)` | windres, gcc/g++, import libs | fails dry-run | rule for `File.obj` | `DeckDll.dll` | low for dry-run |
| `src/drawcardlib/Makefile` | `(cd src/drawcardlib && make -n)` | dlltool, yasm, gcc -m32, objcopy | dry-run prints build plan | tools absent for real build | `Drawcardlib.dll` | low for dry-run |
| `src/cardartlib/Makefile` | `(cd src/cardartlib && make -n)` | gcc/g++, Boost, Windows libs | fails dry-run | rule for `src/main.obj` | `CardArtLib.dll` | low for dry-run |
| `src/manalinkex/Makefile` | `(cd src/manalinkex && make -n)` | gcc -m32, objcopy | dry-run prints build plan | tools/libs not proven | `ManalinkEx.dll` | low for dry-run |
| `src/patches/Makefile` | `(cd src/patches && make -n)` | specific MinGW path | dry-run only | actual guard expects `/d/mingw/bin/gcc` | patch EXEs | medium; patch tools are runtime-mutating |

## Build Conclusions

| Question | Answer |
| --- | --- |
| Can `src/` rebuild shipped binaries today? | no, not proven; top-level `src` dry-run fails. |
| Can `Program/src/` rebuild shipped binaries today? | not proven; dry-run reaches further but required Windows tooling is absent. |
| Are shipped binaries reproducible from source? | not tested and currently unsupported by evidence. |
| Are outputs separated from checked-in runtime binaries? | not consistently; historical helpers can copy into runtime-style paths. |
| Is a native Windows/MinGW environment likely needed? | yes, inferred from makefiles and hardcoded `/d/mingw` guard. |

## Blockers

| Blocker | Evidence | Fix Direction |
| --- | --- | --- |
| Missing/generated header in top-level `src` | `git ls-files` lists `Program/src/card_id.h` only. | Decide whether to regenerate/copy header in a build branch. |
| Missing Windows DLL tools | `tool-availability.txt` shows no `yasm`, `dlltool`, `objcopy`, or `windres`. | Document and install in controlled build environment, not during this audit. |
| Divergent source snapshots | `diff -qr src Program/src` reports many differences. | Establish source provenance before claiming fixes affect shipped runtime. |
| Mutating helper scripts | `src/build.pl` rewrites `ManalinkEh.asm` and copies to `c:\magic2k`. | Add dry-run/out-dir controls before using. |
