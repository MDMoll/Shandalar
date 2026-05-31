# Security and Antivirus Notes

No antivirus or malware scan was run during this documentation pass. Nothing in
these docs should be read as a safety claim about any `.exe`, `.dll`, or archive
in this repository.

Old Windows game binaries, no-CD patches, packed executables, and fan-maintained
DLLs can produce false positives. A false positive is possible, but it is not
proof that a file is safe. Record the scanner, version, date, file path, hash,
and result before making any claim.

## What to Scan

| File family | Why |
| --- | --- |
| `Program/*.exe`, `Program/*.dll` | Main launch targets and runtime DLLs. |
| Root `*.exe`, root `*.dll` | Some root copies match `Program/`; others differ and need separate results. |
| `Mods/**/*.7z`, `*.zip`, `*.7z` | Archives can contain executables or scripts. |
| Checked-in build tools such as `src/*.exe` | Toolchain binaries are executable code. |

## Local Scan Commands

Use whichever tools are already trusted on the test machine. Do not download
scanner installers into this repo.

| Environment | Example command |
| --- | --- |
| macOS with ClamAV installed | `clamscan -r Program src Mods archive` |
| Windows Defender PowerShell | `Start-MpScan -ScanPath "C:\path\to\Shandalar"` |
| Windows Defender command line | `"%ProgramFiles%\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -File "C:\path\to\Shandalar"` |
| Hash before external review | `shasum -a 256 Program/Shandalar.exe Program/Magic.exe` |

If an online multi-scanner is used, record that files may be uploaded to a third
party. Do not upload files unless that is acceptable for the repo/user context.

## Reporting Template

| Date | Scanner/version | Path | SHA-256 | Result | Notes |
| --- | --- | --- | --- | --- | --- |
| Needs testing | Needs testing | Needs testing | Needs testing | Needs testing | No scan recorded in this pass. |
