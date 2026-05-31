# Native Windows ProcMon Plan

Native Windows testing was not available in this pass. Use this plan on a
Windows VM or machine to determine whether the assertion follows missing files,
registry keys, or load failures.

## Filters

Process filter:

```text
Process Name is Shandalar.exe
OR Process Name is Magic.exe
```

Operations:

```text
CreateFile
Load Image
RegOpenKey
RegQueryValue
RegSetValue
```

Results to inspect:

```text
SUCCESS
NAME NOT FOUND
PATH NOT FOUND
ACCESS DENIED
BUFFER OVERFLOW, only when adjacent evidence makes it relevant
```

## Reproduction Sequence

1. Start Process Monitor capture.
2. Launch `Shandalar.exe` from the intended `Program/` working directory.
3. Start a new game.
4. Choose a starting color.
5. Stop capture immediately when the assertion appears.
6. Export filtered events to CSV.
7. Summarize the last successful and failed file/registry operations before the assertion.

## What to Look For

| Evidence | Interpretation |
| --- | --- |
| Missing `.pic`, `.spr`, `.dat`, `.ini`, `.dll`, palette, or art files just before the assertion | Supports a resource/layout hypothesis. |
| Registry reads for install path, CD path, or display settings | Supports registry/setup expectations. |
| Runtime access to `D:\NewMagic` | Would contradict the compile-time-source-path interpretation. |
| No runtime access to `D:\NewMagic` | Supports `__FILE__`/assertion source path interpretation. |
| Same failure on native Windows | Makes Wine-specific graphics behavior less likely. |
| No failure on native Windows | Makes Wine/CrossOver graphics behavior or bottle settings more likely. |
