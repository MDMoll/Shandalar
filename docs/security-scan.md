# Security and Antivirus Notes

A ClamAV scan has been recorded for the current tracked security-target
inventory. This is useful evidence, but it is not a broad safety guarantee for
old Windows game binaries, fan-maintained DLLs, archives, or any future file
changes.

Old Windows game binaries, no-CD patches, packed executables, and fan-maintained
DLLs can produce false positives. A false positive is possible, but it is not
proof that a file is safe. Record the scanner, version, date, file path, hash,
and result before making any claim.

## Verified on this machine

Run from `/Users/mdmoll/Shandalar/Shandalar` on 2026-05-31.

| Check | Result | Meaning |
| --- | --- | --- |
| `brew install clamav` | Installed ClamAV 1.5.2 and dependencies outside the repo. | This made a real local scanner available without downloading installers into the repository. |
| `command -v clamscan` | `/opt/homebrew/bin/clamscan`. | ClamAV is available locally. |
| `clamscan --version` | `ClamAV 1.5.2/28017/Sun May 31 02:27:13 2026`. | Scanner and daily database version recorded for the scan. |
| `freshclam` | Downloaded and tested `daily.cvd` version 28017, `main.cvd` version 63, and `bytecode.cvd` version 339; output included repeated `ERROR: NULL X509 store` warnings but exited 0 after database tests passed. | Virus definitions were present before scanning; keep the warning with the evidence rather than hiding it. |
| `clamscan --fail-if-cvd-older-than=7 --file-list=/private/tmp/shandalar-security-scan-target-paths-e7074aa0.txt --log=/private/tmp/shandalar-clamscan-e7074aa0.log` | `Scanned files: 241`; `Infected files: 0`; `Known viruses: 3627865`; start `2026:05:31 21:47:52`, end `2026:05:31 21:48:51`. | ClamAV reported no infected files for the exact tracked security-target inventory after the safe cleanup and verifier classification update. See [generated/security-scan/clamav-2026-05-31.md](generated/security-scan/clamav-2026-05-31.md). |
| `command -v spctl` | `/usr/sbin/spctl` | macOS Gatekeeper assessment tooling exists, but it is not an antivirus scanner for Windows PE files. |
| `spctl --assess --type execute --verbose=4 Shandalar.exe` | `Shandalar.exe: internal error in Code Signing subsystem` | No useful safety result for the Windows executable. Do not treat this as a pass or fail. |
| `command -v xprotect` | `/usr/bin/xprotect` | macOS XProtect tooling exists locally, but the available commands do not produce a per-file malware scan report for this Windows PE inventory. |
| `xprotect version` | `Version: 5346 Installed: 2026-05-28 08:53:39 +0000` | Records local XProtect metadata only; this is not a scan result for repo files. |
| `xprotect status` | `XProtect launch scans: disabled`; `XProtect background scans: disabled` | No XProtect launch/background scan evidence can be used as the required named scanner result here. |
| `tools/check-security-scanner-availability.sh --strict` | Reports `/opt/homebrew/bin/clamscan` usable after a real scan and `/opt/homebrew/bin/yara` conditional only; reports `mdatp` missing; reports `/usr/bin/xprotect` and `/usr/sbin/spctl` present but not enough for this gate. | Local scanner availability is now sufficient for ClamAV-based evidence, but YARA would still need a named ruleset if used. |
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
tools/check-security-scanner-availability.sh
tools/print-security-scan-baseline.sh
```

These helpers do not run a scanner. The availability helper only reports which
scanner-related commands are visible locally and which macOS tools are not
enough for this Windows PE security gate. Use them only to reduce transcription
errors when filling the reporting table below.

To validate a local TSV of scanner results against the current inventory and
hashes, use:

```sh
tools/create-security-scan-results-template.sh --output security-scan-results.tsv
tools/record-security-scan-result.sh --confirmed-real-scan --path Shandalar.exe --scanner "Windows Defender" --version "VERSION" --date 2026-05-31 --result "Clean" --notes "MpCmdRun.exe custom scan completed"
tools/verify-security-scan-results.sh --results security-scan-results.tsv
tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all
```

The template helper only writes current paths and hashes. Replace every
`Needs testing` placeholder with the real scanner name, version, date, result,
and notes after a scan; placeholders intentionally fail validation.
The recorder helper can update one exact target row, or all current target rows
with `--all-current-targets --replace-row` after a clean whole-tree scanner
pass. It refuses to run without `--confirmed-real-scan`; that flag is an
evidence reminder, not a scan.

The required TSV header is:

```text
path	sha256	scanner	version	date	result	notes
```

`--require-all` passed locally for `security-scan-results.tsv` after the ClamAV
run. The TSV is an ignored local evidence file; the durable summary is recorded
below and in [generated/security-scan/clamav-2026-05-31.md](generated/security-scan/clamav-2026-05-31.md).
This validator does not run a scanner and does not interpret whether a scanner
finding is safe; it only proves the rows refer to current tracked targets with
matching SHA-256 values and non-placeholder scanner metadata.

## Local Scan Commands

Use whichever tools are already trusted on the test machine. Do not download
scanner installers into this repo.

| Environment | Example command |
| --- | --- |
| Local availability check | `tools/check-security-scanner-availability.sh` |
| macOS with ClamAV installed | `clamscan -r Program src Mods archive` |
| Windows Defender PowerShell | `Start-MpScan -ScanPath "C:\path\to\Shandalar"` |
| Windows Defender command line | `"%ProgramFiles%\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -File "C:\path\to\Shandalar"` |
| Hash target inventory before external review | `tools/list-security-scan-targets.sh > scan-targets.tsv` |
| Create scanner result TSV template | `tools/create-security-scan-results-template.sh --output security-scan-results.tsv` |
| Record one scanner result row after a real scan | `tools/record-security-scan-result.sh --confirmed-real-scan --path Shandalar.exe --scanner "Windows Defender" --version "VERSION" --date 2026-05-31 --result "Clean" --notes "MpCmdRun.exe custom scan completed"` |
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
| 2026-05-31 | ClamAV `1.5.2/28017/Sun May 31 02:27:13 2026` | 241 tracked security-scan targets from `tools/list-security-scan-targets.sh` | Validated locally by `tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all` | Clean | `clamscan --fail-if-cvd-older-than=7 --file-list=/private/tmp/shandalar-security-scan-target-paths-e7074aa0.txt --log=/private/tmp/shandalar-clamscan-e7074aa0.log` reported `Scanned files: 241` and `Infected files: 0`. |
