# Generated Investigation Notes

This folder preserves command output and generated evidence from local
reverse-engineering and CrossOver/Wine investigations.

## Use

| Path | Meaning |
| --- | --- |
| `code-audit/` | Generated inventories and command-output snapshots from the static/source/tooling health audit. |
| `create-dibsection-investigation/` | Evidence gathered while diagnosing the start-color `WM_CREATE CreateDIBSection` assertion and related runtime layout questions. |
| `duplicate-cleanup/` | Generated duplicate-group classifications, static reference notes, quarantine results, and removal evidence for the verified duplicate cleanup pass. |
| `install-root-inventory.tsv` | Install-root inventory with launchers, local folders, evidence, size, and status. |
| `install-root-decision.tsv` | Keep/archive/remove decision table for each install root. |
| `manual-gameplay/` | Cropped screenshots and notes from visible gameplay checks. These are narrow evidence snapshots, not full gameplay certification. |
| `safe-cleanup/` | Historical tracked-only duplicate data, candidate classifications, and before/after cleanup metrics for the earlier safe duplicate cleanup pass. Some paths there were removed by later install-root cleanup; use `duplicate-cleanup/` and the install-root TSVs for current state. |
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
