#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

strict=0

usage() {
  cat <<'EOF'
Usage: tools/check-security-scanner-availability.sh [options]

Reports scanner-related tools visible on this machine. This does not run a
malware scan and does not create security evidence for the share gate.

Options:
  --strict   Exit nonzero when no locally usable scanner command is found.
  -h, --help Show this help.
EOF
}

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

tool_path() {
  command -v "$1" 2>/dev/null || true
}

tool_output() {
  if [ -z "$1" ]; then
    printf 'missing'
    return
  fi

  shift
  "$@" 2>&1 | head -5 | tr '\n' '; ' | sed 's/[; ][; ]*$//'
}

markdown_row() {
  printf '| %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --strict)
      strict=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      fail "unknown option: $1"
      ;;
  esac
  shift
done

clamscan_path="$(tool_path clamscan)"
freshclam_path="$(tool_path freshclam)"
yara_path="$(tool_path yara)"
mdatp_path="$(tool_path mdatp)"
xprotect_path="$(tool_path xprotect)"
spctl_path="$(tool_path spctl)"

usable_count=0
[ -n "$clamscan_path" ] && usable_count=$((usable_count + 1))
[ -n "$mdatp_path" ] && usable_count=$((usable_count + 1))
strict_failed=0

printf '# Security Scanner Availability\n\n'
printf 'This helper only reports local tool availability. It does not scan files, upload files, or prove that any binary is safe.\n\n'
printf '| Tool | Path | Gate use | Evidence |\n'
printf '| --- | --- | --- | --- |\n'

if [ -n "$clamscan_path" ]; then
  markdown_row "ClamAV clamscan" "\`$clamscan_path\`" "Usable after a real scan" "\`$(tool_output "$clamscan_path" "$clamscan_path" --version)\`"
else
  markdown_row "ClamAV clamscan" "missing" "Not usable here" "Install or run on another trusted machine; do not download installers into this repo."
fi

if [ -n "$freshclam_path" ]; then
  markdown_row "ClamAV freshclam" "\`$freshclam_path\`" "Definition updater only" "\`$(tool_output "$freshclam_path" "$freshclam_path" --version)\`"
else
  markdown_row "ClamAV freshclam" "missing" "Not a scanner by itself" "Useful only with ClamAV installed."
fi

if [ -n "$mdatp_path" ]; then
  markdown_row "Microsoft Defender mdatp" "\`$mdatp_path\`" "Usable after a real scan" "\`$(tool_output "$mdatp_path" "$mdatp_path" version)\`"
else
  markdown_row "Microsoft Defender mdatp" "missing" "Not usable here" "Use Windows Defender on Windows or Microsoft Defender for Endpoint where installed."
fi

if [ -n "$yara_path" ]; then
  markdown_row "YARA" "\`$yara_path\`" "Conditional only" "\`$(tool_output "$yara_path" "$yara_path" --version)\`; record the named ruleset if used."
else
  markdown_row "YARA" "missing" "Not usable here" "A YARA result would also need a named ruleset and version."
fi

if [ -n "$xprotect_path" ]; then
  markdown_row "macOS XProtect" "\`$xprotect_path\`" "Not enough for this gate" "\`$(tool_output "$xprotect_path" "$xprotect_path" version)\`"
else
  markdown_row "macOS XProtect" "missing" "Not enough for this gate" "XProtect is not the required per-file Windows PE scanner evidence."
fi

if [ -n "$spctl_path" ]; then
  markdown_row "macOS spctl" "\`$spctl_path\`" "Not enough for this gate" "Gatekeeper assessment is not a malware scan for this Windows PE inventory."
else
  markdown_row "macOS spctl" "missing" "Not enough for this gate" "Gatekeeper assessment is not a malware scan for this Windows PE inventory."
fi

if [ "$strict" = "1" ]; then
  printf '\n## Strict Result\n\n'
  if [ "$usable_count" -eq 0 ]; then
    printf 'FAIL: no locally usable scanner command found; run the scan on another trusted machine or install a scanner outside this repo\n'
    strict_failed=1
  else
    printf 'ok: found %s locally usable scanner command(s)\n' "$usable_count"
  fi
fi

printf '\n## Next Commands\n\n'
printf '```sh\n'
printf 'tools/list-security-scan-targets.sh > scan-targets.tsv\n'
printf 'tools/create-security-scan-results-template.sh --output security-scan-results.tsv\n'
if [ -n "$clamscan_path" ]; then
  printf 'clamscan -r . > clamscan-report.txt\n'
fi
if [ -n "$mdatp_path" ]; then
  printf 'mdatp scan custom --path "%s" > mdatp-report.txt\n' "$repo_root"
fi
printf 'tools/record-security-scan-result.sh --confirmed-real-scan --all-current-targets --replace-row --scanner "SCANNER" --version "VERSION" --date %s --result "Clean" --notes "COMMAND completed with no detections"\n' "$(date +%F)"
printf 'tools/verify-security-scan-results.sh --results security-scan-results.tsv --require-all\n'
printf '```\n'

exit "$strict_failed"
