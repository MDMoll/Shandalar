# Memory-Safety Audit

This is a source-level audit. It does not prove current shipped binaries contain
or do not contain the same code, unless a path is already tied to documented
patch bytes.

Raw evidence:
[generated/code-audit/grep-hazard-results.tsv](generated/code-audit/grep-hazard-results.tsv).

| ID | Path | Function/Area | Risk | Evidence | Severity | Confidence | Proposed Fix | Test |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MS001 | `src/cardartlib/src/main.cpp` | `loadDat()` | unchecked file, read, allocation, and index assumptions | `fopen`, `fread`, `malloc`, and `idToName[id]` are used without local validation | high | medium | Add checked reads, allocation checks, and id bounds | malformed `Cards.dat` harness after build proof |
| MS002 | `src/cardartlib/src/main.cpp` | `idToNameFun()` | fixed-size path buffer overflow | `sprintf(tmp, "CardArtManalink/%s...")` into `char tmp[254]` | medium | medium | Use `snprintf`; reject missing or out-of-range ids | long-name synthetic data |
| MS003 | `src/drawcardlib/config.c` | `get_pichandlename()` | fixed static buffer overflow | `sprintf()` into `char buf[3][160]` with base/config names | medium | medium | Use bounded formatting and larger path model | long install path/config test |
| MS004 | `src/drawcardlib/config.c` | `get_cfg_int2_raw()` | key concatenation overflow | `strcpy`/`strcat` into `char buf[104]` | medium | medium | Use `snprintf` | long key config test |
| MS005 | `src/cards/draft.c` | `load_custom_set()` | unbounded input parse | `fscanf(file, "%[^\n]", buffer)` and `%[0-9]` with fixed buffers | medium | high | Use `fgets` and bounded parsing; check `fopen` | malformed `DraftSets` copy |
| MS006 | `src/functions/functions.c` | `damage_dealt_*_arbitrary()` | assert-only unsupported modes | `ASSERT(... "unimplemented")` for several flags | medium | high | Add tested fallback or guard callers | scenario tests for each flag |
| MS007 | `src/functions/has_creature_type.c` | subtype damage helpers | assert-only unsupported modes | `ASSERT(... "unimplemented for subtype_deals_damage")` | medium | high | Same as above | card tests reaching subtype damage modes |
| MS008 | `src/drawcardlib/` and `src/deck/deckdll.cpp` | GDI object/DC lifetime | leaks or invalid selected-object deletion | many `CreateCompatibleDC`, `SelectObject`, `DeleteObject`, `DeleteDC`, `ReleaseDC` sites | medium | medium | Audit restore/delete pairs one module at a time | Windows GDI handle monitoring |
| MS009 | `Shandalar.exe` / `FaceMaker.exe` | `CreateDIBSection` wrappers | binary-patched crash path | documented `hSection = NULL` patches exist | high | high | Do not change without binary rollback/test plan | `xxd` patch checks plus manual start-color tests |
| MS010 | `Magic.exe` | declared-attacker undo cave | gameplay-state consistency | patch clears `STATE_ATTACKING`; edge costs/triggers unproven | medium | medium | Keep as manual-test-gated | manual attacker edge cases |

## Interpretation

Most findings are `source-level suspicion only` because the build is not
reproducible. The strongest current runtime-backed memory-safety item is the
`CreateDIBSection` family because patch bytes and smoke evidence are already
documented.
