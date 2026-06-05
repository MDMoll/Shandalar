# Optimization Opportunities

These are practical opportunities, not performance claims. No profiler was run.
Machine-readable list:
[generated/code-audit/optimization-candidates.tsv](generated/code-audit/optimization-candidates.tsv).

| ID | Area | Current Behavior | Cost/Risk | Suggested Optimization | Evidence Needed | Fix Risk |
| --- | --- | --- | --- | --- | --- | --- |
| O001 | audit reports | Inventories still cover the broad repo, but grep-style reports now default to focused source/tooling scans. | Full scans can still be noisy when requested with `--scan-scope all`. | Add more named scopes only if future reports need them. | Compare focused and all-scope report size when needed. | safe |
| O002 | build system | Two source snapshots have overlapping but divergent files; current mirrored source-safety edits now have parity-marker checks. | Duplicated maintenance work and source-to-runtime uncertainty. | Choose an authoritative build source or define a sync workflow that keeps the parity guard current. | Source provenance, parity checks, and build proof. | moderate |
| O003 | CardArtLib | Art path strings and cache keys are built per lookup. | Possible overhead with huge art trees. | Cache resolved art metadata after `Cards.dat` load. | Profile card-art lookup. | moderate |
| O004 | drawcardlib | Config/path work repeats across render setup. | Startup/render overhead. | Parse and cache config/path state per config reload. | Render timing before/after. | moderate |
| O005 | deck builder | Deck dialogs scan/build paths over large deck trees. | Slow dialog open with many decks. | Cache directory listings during one dialog/session. | Deck dialog timing. | moderate |
| O006 | draft tooling | Draft mode writes and rereads runtime-local files; failed output opens are now guarded. | Disk churn and clutter remain. | Route generated draft artifacts to a configured output directory after behavior is proven in a disposable copy. | Disposable draft-mode test. | moderate |
| O007 | maintenance tooling | The audit helper now has focused scans; other duplicate/security/branch scans may still traverse many files. | Maintenance commands can be slow on 52k+ files. | Add focused scopes and size prefilters to other tools where appropriate. | Command timing. | safe |
| O008 | security scan workflow | Local TSV must refresh after scanner-target edits. | Correct but easy to forget. | Improve docs/tool messaging around scanner-target changes. | Verification failure/recovery test. | safe |
| O009 | GDI drawing | GDI object creation/restoration is scattered. | Leak/performance risk in long sessions. | Centralize restore/delete helpers one module at a time. | Windows GDI handle monitoring. | moderate |
| O010 | card prompts | Many prompt buffers are formatted ad hoc. | Maintainability and buffer risk. | Add prompt-format helpers after build/test harness exists. | Card prompt regression tests. | risky |
