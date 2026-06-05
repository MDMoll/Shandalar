# Build Health

This audit used dry-run probes only. No shipped binaries were rebuilt or
overwritten.

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
| `yasm`, `dlltool`, `objcopy`, `windres`, `nasm` | absent | verified |
| `cppcheck`, `scan-build`, `clang-tidy`, `infer`, `semgrep` | absent | verified |

## Build Probe Results

| Build Target | Command | Required Tools | Result | Missing Inputs | Output Expected | Risk |
| --- | --- | --- | --- | --- | --- | --- |
| `src/Makefile` | `(cd src && make -n)` | GNU make, gcc/g++, yasm, dlltool, objcopy | dry-run prints build plan | Windows DLL toolchain absent for real build | `ManalinkEh.dll` | low for dry-run; high for real build |
| `Program/src/Makefile` | `(cd Program/src && make -n)` | GNU make, gcc/g++, yasm, dlltool, objcopy | dry-run prints build plan | Windows DLL toolchain absent for real build | `ManalinkEh.dll` | low for dry-run; high for real build |
| `src/deck/Makefile` | `(cd src/deck && make -n)` | windres, gcc/g++, import libs | fails dry-run | rule for `File.obj` | `DeckDll.dll` | low for dry-run |
| `src/drawcardlib/Makefile` | `(cd src/drawcardlib && make -n)` | dlltool, yasm, gcc -m32, objcopy | dry-run prints build plan | tools absent for real build | `Drawcardlib.dll` | low for dry-run |
| `src/cardartlib/Makefile` | `(cd src/cardartlib && make -n)` | gcc/g++, Boost, Windows libs | dry-run prints build plan | Windows/Boost DLL toolchain absent for real build | `CardArtLib.dll` | low for dry-run; high for real build |
| `Program/src/cardartlib/Makefile` | `(cd Program/src/cardartlib && make -n)` | gcc/g++, Boost, Windows libs | dry-run prints build plan | Windows/Boost DLL toolchain absent for real build | `CardArtLib.dll` | low for dry-run; high for real build |
| `src/manalinkex/Makefile` | `(cd src/manalinkex && make -n)` | gcc -m32, objcopy | dry-run prints build plan | tools/libs not proven | `ManalinkEx.dll` | low for dry-run |
| `src/patches/Makefile` | `(cd src/patches && make -n)` | specific MinGW path | dry-run only | actual guard expects `/d/mingw/bin/gcc` | patch EXEs | medium; patch tools are runtime-mutating |

## Build Conclusions

| Question | Answer |
| --- | --- |
| Can `src/` rebuild shipped binaries today? | no, not proven; top-level `src` dry-run now reaches a full build plan, but required Windows DLL tools are absent and no real build was attempted. |
| Can `Program/src/` rebuild shipped binaries today? | not proven; dry-run reaches further but required Windows tooling is absent. |
| Are shipped binaries reproducible from source? | not tested and currently unsupported by evidence. |
| Are outputs separated from checked-in runtime binaries? | not consistently; historical helpers can copy into runtime-style paths. |
| Is a native Windows/MinGW environment likely needed? | yes, inferred from makefiles and hardcoded `/d/mingw` guard. |

## Blockers

| Blocker | Evidence | Fix Direction |
| --- | --- | --- |
| Missing/generated header in top-level `src` | mitigated; `src/card_id.h` was restored as an exact copy of `Program/src/card_id.h` (`22fd41e536d24d5bdbbcd5ef6b4003e420aed135066a033dc4c1d6dac24675c1`). | Keep the copied header until a true generator/provenance path is found. |
| Missing Windows DLL tools | `tool-availability.txt` shows no `yasm`, `dlltool`, `objcopy`, or `windres`. | Document and install in controlled build environment, not during this audit. |
| Divergent source snapshots | `tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only` records 80 matching paths, 142 differing paths, and 100 paths missing from `Program/src`; current exact-match files and safety markers pass. | Keep the parity guard, then establish source provenance before claiming fixes affect shipped runtime. |
| Mutating helper scripts | `src/build.pl` can rewrite `ManalinkEh.asm` when intentionally invoked with a card name; `src/deploy.bat` confirmed mode copies/deletes under `c:\magic2k`. | Use `--dry-run` or explicit confirmation only in controlled build/packaging copies. |

## Fixes Applied After Audit

| Fix | Evidence | Remaining Limit |
| --- | --- | --- |
| Added `tools/check-build-prereqs.sh`. | `tools/check-build-prereqs.sh --report-only` reports the restored matching `src/card_id.h`, missing Windows build tools, and source provenance warning without building or copying. | It is a preflight, not a full build repair. Current machine still lacks the full toolchain. |
| Hardened `src/build.pl`. | `perl -c src/build.pl` and `perl src/build.pl --help`; legacy `c:\magic2k` copies now require `--legacy-copy-targets` or explicit `--copy-to`. | The script can still edit `ManalinkEh.asm` when intentionally run with a card name and without `--dry-run`. Use in a disposable/build branch. |
| Restored top-level `src/card_id.h`. | `shasum -a 256 Program/src/card_id.h src/card_id.h`; `(cd src && make -n)` now prints the full `ManalinkEh.dll` plan. | Header provenance still traces to `Program/src`; a true generator was not found. |
| Added source snapshot parity reporting. | `tools/check-source-snapshot-parity.sh` records full snapshot divergence as evidence while enforcing exact-match files and mirrored safety markers for current source fixes. | It guards against accidental drift in the current safety pass; it does not choose the authoritative source snapshot or prove rebuild reproducibility. |
| Guarded draft output file handling. | `src/cards/draft.c` and `Program/src/cards/draft.c` use `open_draft_output()` for runtime-local draft logs/deck writes and remain exact matches. | Output paths still depend on the runtime working directory; relocation needs a draft-mode copy test. |
