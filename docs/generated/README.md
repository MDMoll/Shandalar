# Generated Investigation Notes

This folder preserves command output and generated evidence from local
reverse-engineering and CrossOver/Wine investigations.

## Use

| Path | Meaning |
| --- | --- |
| `create-dibsection-investigation/` | Evidence gathered while diagnosing the start-color `WM_CREATE CreateDIBSection` assertion and related runtime layout questions. |
| `manual-gameplay/` | Cropped screenshots and notes from visible gameplay checks. These are narrow evidence snapshots, not full gameplay certification. |
| `safe-cleanup/` | Tracked-only duplicate data, candidate classifications, and before/after cleanup metrics for the safe duplicate cleanup pass. |
| `security-scan/` | Summaries of local scanner evidence; raw local scanner reports may remain outside the repo when they are machine-specific. |

## Policy

| Rule | Why |
| --- | --- |
| Treat files here as evidence snapshots, not primary docs. | The concise, maintained docs live one level up in `docs/`. |
| Do not edit generated output to make conclusions look cleaner. | The value is in preserving what commands actually produced. |
| If output is regenerated, record the command and date in the focused investigation doc. | Future readers need to separate old evidence from new evidence. |

Start with [../troubleshooting.md](../troubleshooting.md),
[../crossover-macos.md](../crossover-macos.md), or
[../runtime-testing-policy.md](../runtime-testing-policy.md), or
[../bugs/create-dibsection-after-color.md](../bugs/create-dibsection-after-color.md)
before reading these files.
