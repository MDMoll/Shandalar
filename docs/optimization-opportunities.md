# Optimization Opportunities

These are practical opportunities, not performance claims. No profiler was run.
Machine-readable list:
[generated/code-audit/optimization-candidates.tsv](generated/code-audit/optimization-candidates.tsv).

| ID | Area | Current Behavior | Cost/Risk | Suggested Optimization | Evidence Needed | Fix Risk |
| --- | --- | --- | --- | --- | --- | --- |
| O001 | audit reports | Regex scans cover the broad repo while skipping local ignored files and the helper output directory. | Large reports can still be noisy. | Add focused source/tooling scopes by default while keeping all-file mode available. | Compare report size and useful matches. | safe |
| O002 | build system | Two source snapshots have overlapping but divergent files. | Duplicated maintenance work. | Choose an authoritative build source or define a sync workflow. | Source provenance and build proof. | moderate |
| O003 | CardArtLib | Art path strings and cache keys are built per lookup. | Possible overhead with huge art trees. | Cache resolved art metadata after `Cards.dat` load. | Profile card-art lookup. | moderate |
| O004 | drawcardlib | Config/path work repeats across render setup. | Startup/render overhead. | Parse and cache config/path state per config reload. | Render timing before/after. | moderate |
| O005 | deck builder | Deck dialogs scan/build paths over large deck trees. | Slow dialog open with many decks. | Cache directory listings during one dialog/session. | Deck dialog timing. | moderate |
| O006 | draft tooling | Draft mode writes and rereads runtime-local files. | Disk churn and clutter. | Route generated draft artifacts to a configured output directory. | Disposable draft-mode test. | moderate |
| O007 | maintenance tooling | Duplicate/security/branch scans traverse many files. | Slow maintenance commands. | Add focused scopes and size prefilters where appropriate. | Command timing. | safe |
| O008 | security scan workflow | Local TSV must refresh after scanner-target edits. | Correct but easy to forget. | Improve docs/tool messaging around scanner-target changes. | Verification failure/recovery test. | safe |
| O009 | GDI drawing | GDI object creation/restoration is scattered. | Leak/performance risk in long sessions. | Centralize restore/delete helpers one module at a time. | Windows GDI handle monitoring. | moderate |
| O010 | card prompts | Many prompt buffers are formatted ad hoc. | Maintainability and buffer risk. | Add prompt-format helpers after build/test harness exists. | Card prompt regression tests. | risky |
