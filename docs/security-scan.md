# Security and Antivirus Notes

No antivirus or malware scan has been recorded for this branch. Nothing in
these docs should be read as a safety claim about any `.exe`, `.dll`, or archive
in this repository.

Old Windows game binaries, no-CD patches, packed executables, and fan-maintained
DLLs can produce false positives. A false positive is possible, but it is not
proof that a file is safe. Record the scanner, version, date, file path, hash,
and result before making any claim.

## Verified on this machine

Run from `/Users/mdmoll/Shandalar/Shandalar` on 2026-05-31.

| Check | Result | Meaning |
| --- | --- | --- |
| `command -v clamscan` | No path printed. | ClamAV is not installed or not on `PATH` here, so no ClamAV scan was run. |
| `command -v spctl` | `/usr/sbin/spctl` | macOS Gatekeeper assessment tooling exists, but it is not an antivirus scanner for Windows PE files. |
| `spctl --assess --type execute --verbose=4 Shandalar.exe` | `Shandalar.exe: internal error in Code Signing subsystem` | No useful safety result for the Windows executable. Do not treat this as a pass or fail. |
| `file Shandalar.exe Program/Magic.exe ManalinkEh.dll Program/ManalinkEh.dll` | PE32 Windows executable/DLL files. | Confirms the active targets are Windows PE files, so use a scanner that supports that format. |
| `shasum -a 256 Shandalar.exe Program/Shandalar.exe Magic.exe Program/Magic.exe FaceMaker.exe Program/FaceMaker.exe ManalinkEh.dll Program/ManalinkEh.dll` | Hashes recorded in [runtime-manifest.md](runtime-manifest.md), [running.md](running.md), and [magic-exe.md](magic-exe.md). | Hashes identify exactly which patched binaries still need scan results. |

## What to Scan

| File family | Why |
| --- | --- |
| `Program/*.exe`, `Program/*.dll` | Main launch targets and runtime DLLs. |
| Root `*.exe`, root `*.dll` | Some root copies match `Program/`; others differ and need separate results. |
| `Mods/**/*.7z`, `*.zip`, `*.7z` | Archives can contain executables or scripts. |
| Checked-in build tools such as `src/*.exe` | Toolchain binaries are executable code. |
| Checked-in scripts such as `*.bat`, `*.cmd`, `*.vbs`, `*.pl`, and `*.sh` | Launch helpers and patch/build scripts are executable instructions and should be reviewed/scanned as code. |

For an exact tracked-file inventory, run:

```sh
tools/list-security-scan-targets.sh
```

Current maintained target kinds include PE executables, PE DLLs, archives,
Windows scripts, Java archives, Perl scripts, and shell scripts. The output is
tab-separated: path, kind, byte count, and SHA-256.

Before or after a real scanner pass, print a Markdown-ready evidence baseline
with the current branch, target counts, priority hashes, and commands:

```sh
tools/print-security-scan-baseline.sh
```

This helper does not run a scanner. Use it only to reduce transcription errors
when filling the reporting table below.

To validate a local TSV of scanner results against the current inventory and
hashes, use:

```sh
tools/create-security-scan-results-template.sh --output security-scan-results.tsv
tools/verify-security-scan-results.sh --results security-scan-results.tsv
tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all
```

The template helper only writes current paths and hashes. Replace every
`Needs testing` placeholder with the real scanner name, version, date, result,
and notes after a scan; placeholders intentionally fail validation.

The required TSV header is:

```text
path	sha256	scanner	version	date	result	notes
```

`--require-all` must pass before treating the security-scan gate as complete.
This validator does not run a scanner and does not interpret whether a scanner
finding is safe; it only proves the rows refer to current tracked targets with
matching SHA-256 values and non-placeholder scanner metadata.

## Local Scan Commands

Use whichever tools are already trusted on the test machine. Do not download
scanner installers into this repo.

| Environment | Example command |
| --- | --- |
| macOS with ClamAV installed | `clamscan -r Program src Mods archive` |
| Windows Defender PowerShell | `Start-MpScan -ScanPath "C:\path\to\Shandalar"` |
| Windows Defender command line | `"%ProgramFiles%\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -File "C:\path\to\Shandalar"` |
| Hash target inventory before external review | `tools/list-security-scan-targets.sh > scan-targets.tsv` |
| Create scanner result TSV template | `tools/create-security-scan-results-template.sh --output security-scan-results.tsv` |
| Print Markdown evidence baseline | `tools/print-security-scan-baseline.sh` |
| Validate recorded scan rows | `tools/verify-security-scan-results.sh --results security-scan-results.tsv` |
| Validate full target coverage | `tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all` |

`scan-targets.tsv`, `security-scan-results.tsv`, `clamscan-report.txt`,
`windows-defender-report.txt`, `*.bundle`, and `*.bundle.sha256` are ignored
local handoff files. Record durable scanner conclusions in this doc instead of
committing raw local report files.

If an online multi-scanner is used, record that files may be uploaded to a third
party. Do not upload files unless that is acceptable for the repo/user context.

## Reporting Template

| Date | Scanner/version | Path | SHA-256 | Result | Notes |
| --- | --- | --- | --- | --- | --- |
| Needs testing | Needs testing | Needs testing | Needs testing | Needs testing | No antivirus or malware scanner result is recorded for this branch. |
