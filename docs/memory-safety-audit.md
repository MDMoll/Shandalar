# Memory-Safety Audit

This is a source-level audit. It does not prove current shipped binaries contain
or do not contain the same code, unless a path is already tied to documented
patch bytes.

Raw evidence:
[generated/code-audit/grep-hazard-results.tsv](generated/code-audit/grep-hazard-results.tsv).

| ID | Path | Function/Area | Risk | Evidence | Severity | Confidence | Proposed Fix | Test |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MS001 | `src/cardartlib/src/main.cpp`; `Program/src/cardartlib/src/main.cpp` | `loadDat()` | mitigated unchecked file, read, allocation, and index assumptions | checks `fopen`, header reads, data reads, allocation, card ids, name offsets, string termination, and missing names before art lookup | low | medium | Add malformed `Cards.dat` harness after build/test support exists | static inspection and CardArtLib dry-runs now; malformed `Cards.dat` copy later |
| MS002 | `src/cardartlib/src/main.cpp`; `Program/src/cardartlib/src/main.cpp` | `idToNameFun()` and image counting | mitigated fixed-size path/name buffer overflow | replaces `char tmp[254]` and `char cardname[200]` formatting with `std::string` path/name construction and id/name validation | low | medium | Add long-name art harness after build/test support exists | static inspection and CardArtLib dry-runs now; long-name art copy later |
| MS003 | `src/drawcardlib/config.c`; `Program/src/drawcardlib/config.c` | `get_pichandlename()` | mitigated fixed static buffer overflow | replaces `sprintf()` into `char buf[3][160]` with larger static diagnostic buffers and `snprintf` | low | medium | Add long install path/config-name harness after build/test support exists | static grep and drawcardlib dry-runs now; long-path copy later |
| MS004 | `src/drawcardlib/config.c`; `Program/src/drawcardlib/config.c` | `get_cfg_int2_raw()` and font key lookup | mitigated key concatenation overflow | replaces `strcpy`/`strcat` into `char buf[104]` with allocated INI key construction and fallback behavior on allocation failure | low | medium | Add long key config harness after build/test support exists | static grep and drawcardlib dry-runs now; long-key config copy later |
| MS005 | `src/cards/draft.c`; `Program/src/cards/draft.c` | `load_custom_set()` | mitigated unbounded input parse | replaced `%[` `fscanf` scans with checked `fopen`, bounded `fgets`, line stripping, set-slot guards, card-count guards, and persistent set-name storage | low | high | Add malformed `DraftSets` harness after build/test support exists | static grep and source dry-runs now; malformed `DraftSets` copy later |
| MS012 | `src/cards/draft.c`; `Program/src/cards/draft.c` | draft output logging and deck writes | mitigated unchecked output handles | `open_draft_output()` guards `draft_errors.txt`, `picks.txt`, `packs.txt`, and deck-file writes; invalid AI picks are skipped before score-log dereference | low | high | Keep runtime-CWD output behavior documented; consider relocation only after draft-mode copy test | static grep and parity checks now; draft-mode disposable-copy test later |
| MS006 | `src/functions/functions.c` | `damage_dealt_*_arbitrary()` | assert-only unsupported modes | `ASSERT(... "unimplemented")` for several flags | medium | high | Add tested fallback or guard callers | scenario tests for each flag |
| MS007 | `src/functions/has_creature_type.c` | subtype damage helpers | assert-only unsupported modes | `ASSERT(... "unimplemented for subtype_deals_damage")` | medium | high | Same as above | card tests reaching subtype damage modes |
| MS008 | `src/drawcardlib/` and `src/deck/deckdll.cpp` | GDI object/DC lifetime | leaks or invalid selected-object deletion | many `CreateCompatibleDC`, `SelectObject`, `DeleteObject`, `DeleteDC`, `ReleaseDC` sites | medium | medium | Audit restore/delete pairs one module at a time | Windows GDI handle monitoring |
| MS009 | `Shandalar.exe` / `FaceMaker.exe` | `CreateDIBSection` wrappers | binary-patched crash path | documented `hSection = NULL` patches exist | high | high | Do not change without binary rollback/test plan | `xxd` patch checks plus manual start-color tests |
| MS010 | `Magic.exe` | declared-attacker undo cave | gameplay-state consistency | patch clears `STATE_ATTACKING`; edge costs/triggers unproven | medium | medium | Keep as manual-test-gated | manual attacker edge cases |
| MS011 | `src/functions/targets.c`; `Program/src/functions/targets.c` | auto-target text loading | mitigated unbounded input parse | replaced `%[` `fscanf` scans with checked `fopen`, bounded `fgets`, numeric-line parsing, and `auto_targets` sentinel-space guard | low | high | Add malformed `TargetsHuman.txt`/`TargetsAI.txt` harness after build/test support exists | static grep and source dry-runs now; malformed target-list copy later |

## Interpretation

Most findings are `source-level suspicion only` because the build is not
reproducible. The strongest current runtime-backed memory-safety item is the
`CreateDIBSection` family because patch bytes and smoke evidence are already
documented.
