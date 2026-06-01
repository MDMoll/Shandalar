# Bug Backlog

This backlog is evidence-based triage, not a claim that every item is a
runtime bug. Machine-readable source:
[generated/code-audit/potential-bugs.tsv](generated/code-audit/potential-bugs.tsv).

| Rank | ID | Title | Severity | Confidence | Area | Evidence | Proposed Fix | Test | Fix Risk |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | B001 | Top-level `src` dry-run cannot reach `ManalinkEh.dll` build | high | high | build | `src-make-dry-run.txt` exits at `functions/utility.obj`; `src/card_id.h` absent | Document/repair build prerequisites in a build-only branch | `make -n`, then isolated build output comparison | safe |
| 2 | B002 | Windows-oriented build tools absent | high | high | build | no `yasm`, `dlltool`, `objcopy`, `windres` | Document exact toolchain; keep outputs separate | tool availability plus dry-run | safe |
| 3 | B003 | `src` and `Program/src` are divergent snapshots | high | high | source provenance | `diff -qr` reports many differences; `Program/src/card_id.h` only | Choose/record authoritative source snapshot | source diff and eventual build proof | safe |
| 4 | B006 | `CardArtLib` `Cards.dat` loader lacks checks | high | medium | memory safety | `loadDat()` has unchecked `fopen`, `fread`, `malloc` | Add checks in source after build harness exists | malformed data harness | moderate |
| 5 | B012 | `Program/Shandalar.exe` layout lacks adjacent `zlib.dll` in `MTG` | high | high | runtime path | existing logged failure in docs | Keep root launch primary; test `Program` in copy | exact-path launch in disposable copy | safe |
| 6 | B010 | Draft custom-set parser uses unbounded `fscanf` | medium | high | data parsing | `src/cards/draft.c` reads `%[^\n]` into fixed buffers | Width-limit or line-based parser | malformed DraftSets copy | moderate |
| 7 | B004 | `src/build.pl` mutates source and copies to `c:\magic2k` | medium | high | tooling | script rewrites `ManalinkEh.asm` and copies DLL | Add `--dry-run`/`--out` guard | syntax and dry-run tests | moderate |
| 8 | B005 | `src/deploy.bat` deletes/copies under `c:\magic2k` | medium | high | tooling | hardcoded `del /s/q` and many `copy` commands | Document as historical packaging script | static review | safe |
| 9 | B013 | Root and `Program` `Magic.exe` differ but both matter | medium | high | runtime path | different hashes and separate launch paths | Always patch/test both exact paths | hash and launch matrix | safe |
| 10 | B014 | Root and `Program` `ManalinkEh.dll` differ by hash/offset | medium | high | patch risk | healer patch offsets differ | Path-specific hash/offset verification | `xxd`/`shasum` | safe |
| 11 | B015 | Declared-attacker undo may not reverse costs/triggers | medium | medium | gameplay state | docs note attack hooks can run before selection completes | Keep manual-test gate; avoid more patching | gameplay R3/R4 and edge cases | do-not-change-yet |
| 12 | B016 | Damage-prevention patch may not cover all handlers | medium | medium | gameplay state | current patch covers Samite/Femeref/Kithkin family | Inventory other prevention handlers first | manual repro by card | do-not-change-yet |
| 13 | B017 | Unimplemented DDBM/subtype flags assert | medium | high | crash risk | `ASSERT(... "unimplemented")` in source | Convert only with tests | targeted scenario tests | risky |
| 14 | B018 | GDI lifetime is hand-managed in many UI paths | medium | medium | resource lifetime | repeated `CreateCompatibleDC`, `SelectObject`, `DeleteObject` | Audit one module at a time | Windows GDI handle monitoring | moderate |
| 15 | B007 | `CardArtLib` path `sprintf` can overflow | medium | medium | memory safety | `char tmp[254]` plus card name from data | Use `snprintf` and id validation | long-name harness | moderate |
| 16 | B008 | `drawcardlib` frame-name formatting can overflow | medium | medium | memory safety | static `char buf[3][160]` plus base/config names | Use checked formatting | long-path test | moderate |
| 17 | B009 | Config key concatenation can overflow | medium | medium | memory safety | `strcpy`/`strcat` into `char buf[104]` | Use `snprintf` | long-key test | moderate |
| 18 | B019 | Launcher mutates local runtime/mod trees | medium | high | runtime layout | `Manalink_Launcher.cmd` deletes temp, installs mods, deletes draft decks | Use disposable-copy tests for launcher work | before/after tree diff | safe |
| 19 | B011 | Draft mode writes generated files into runtime CWD | low | high | runtime layout | writes `picks.txt`, `packs.txt`, draft decks by relative path | Configure output location | draft-mode copy test | moderate |
| 20 | B020 | Optional static analysis is missing | low | high | tooling | optional analyzers not installed | Add optional analyzer workflow later | analyzer reports | safe |
