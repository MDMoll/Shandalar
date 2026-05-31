#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

expect_file() {
  local path="$1"
  [ -e "$path" ] || fail "missing $path"
}

expect_tracked_file() {
  local path="$1"
  expect_file "$path"
  git ls-files --error-unmatch "$path" >/dev/null || fail "$path is not tracked"
}

expect_hash() {
  local path="$1"
  local expected="$2"
  expect_file "$path"
  local actual
  actual="$(shasum -a 256 "$path" | awk '{print $1}')"
  [ "$actual" = "$expected" ] || fail "$path sha256 $actual != $expected"
}

expect_hex_prefix() {
  local path="$1"
  local offset="$2"
  local length="$3"
  local expected="$4"
  expect_file "$path"
  local actual
  actual="$(xxd -p -l "$length" -s "$offset" "$path" | tr -d '\n')"
  [ "$actual" = "$expected" ] || fail "$path hex at $offset $actual != $expected"
}

if [ "${ALLOW_DIRTY:-0}" != "1" ]; then
  status="$(git status --short --untracked-files=all)"
  [ -z "$status" ] || fail "working tree is not clean; set ALLOW_DIRTY=1 for an in-progress local check"
  pass "working tree is clean"
else
  pass "working tree cleanliness skipped because ALLOW_DIRTY=1"
fi

if [ "${ALLOW_IGNORED_LOCAL:-0}" != "1" ]; then
  ignored_untracked="$(git status --ignored --short --untracked-files=all | awk '/^!! / {print substr($0, 4)}')"
  if [ -n "$ignored_untracked" ]; then
    printf 'FAIL: ignored local files are present; remove them or set ALLOW_IGNORED_LOCAL=1:\n' >&2
    printf '%s\n' "$ignored_untracked" >&2
    exit 1
  fi
  pass "ignored local clutter is absent"
else
  pass "ignored local clutter check skipped because ALLOW_IGNORED_LOCAL=1"
fi

tracked_ignored="$(git ls-files -ci --exclude-standard)"
[ "$tracked_ignored" = "CardArtNew/Thumbs.db" ] || fail "unexpected tracked ignored files: ${tracked_ignored:-<none>}"
pass "tracked ignored file inventory is expected"

for path in \
  scan-targets.tsv \
  security-scan-results.tsv \
  clamscan-report.txt \
  windows-defender-report.txt \
  codex-shandalar-crossover-updates-test.bundle \
  codex-shandalar-crossover-updates-test.bundle.sha256
do
  git check-ignore -q "$path" || fail "$path is not ignored"
done
pass "local generated report/handoff files are ignored"

for path in \
  Shandalar.exe \
  Program/Magic.exe \
  Cards.dat \
  Statwin/statscrn.tmp \
  MAGIC5.SVE \
  MAGIC5.map \
  MAGIC5.fce \
  CardArtNew/Thumbs.db \
  archive/generated-local/Duel.GID \
  Duel.hlp
do
  git check-attr binary -- "$path" | grep -q ': binary: set' || fail "$path is not marked binary"
done
pass "representative binary attributes are set"

for path in \
  MENUBAK.PIC \
  Program/MENUBAK.PIC \
  WINBAK01.PIC \
  WINBAK02.PIC \
  WORLBAK1.PIC \
  Program/WINBAK01.PIC \
  Program/WINBAK02.PIC \
  Program/WORLBAK1.PIC \
  FaceMaker-Original.exe \
  FaceMaker-nores.exe \
  Program/FaceMaker-nores.exe
do
  expect_tracked_file "$path"
  git check-attr binary -- "$path" | grep -q ': binary: set' || fail "$path is not marked binary"
done
pass "protected cleanup false positives are present"

expect_hash Shandalar.exe ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b
expect_hash Program/Shandalar.exe ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b
expect_hash FaceMaker.exe 41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246
expect_hash Program/FaceMaker.exe 41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246
expect_hash FaceMaker-Original.exe 0471afcd0288a07422355ff2af224c40f8b29dc0a864eed90b3399e285f42c7e
expect_hash FaceMaker-nores.exe 43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b
expect_hash Program/FaceMaker-nores.exe 43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b
expect_hash Magic.exe 5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1
expect_hash Program/Magic.exe 0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa
expect_hash ManalinkEh.dll 6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb
expect_hash Program/ManalinkEh.dll 7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517
pass "patched runtime hashes match docs"

expect_hex_prefix Shandalar.exe 0x1785b0 11 6a0057508b4d1051ff7504
expect_hex_prefix Program/Shandalar.exe 0x1785b0 11 6a0057508b4d1051ff7504
expect_hex_prefix FaceMaker.exe 0x5f40 11 6a0057508b4d1051ff7504
expect_hex_prefix Program/FaceMaker.exe 0x5f40 11 6a0057508b4d1051ff7504
expect_hex_prefix Magic.exe 0x3c303 13 e9c0d801009090909090909090
expect_hex_prefix Program/Magic.exe 0x3c303 13 e9c0d801009090909090909090
expect_hex_prefix Magic.exe 0x59bc8 13 f74608040000000f8512000000
expect_hex_prefix Program/Magic.exe 0x59bc8 13 f74608040000000f8512000000
expect_hex_prefix ManalinkEh.dll 0x3bb035 16 f60590f14e00040f84ae000000e90100
expect_hex_prefix Program/ManalinkEh.dll 0x381a25 16 f60590f14e00040f84ae000000e90100
pass "representative binary patch bytes match docs"

save_slot_count="$(git ls-files | awk '/^MAGIC[0-9a-d]\.(SVE|map|fce)$/ {count++} END {print count+0}')"
[ "$save_slot_count" = "33" ] || fail "expected 33 tracked root save slot/map/face files, found $save_slot_count"
save_export_count="$(git ls-files | awk '/^CSV\/MAGIC[3-6]\// {count++} END {print count+0}')"
[ "$save_export_count" = "32" ] || fail "expected 32 tracked CSV save exports, found $save_export_count"
for path in \
  MAGIC5 \
  Savedescs \
  Program/Savedescs \
  FaceMostRecent.txt \
  Screennames/Activename.dat \
  Screennames/CirothUngol.scn \
  Screennames/Player.scn \
  Manalink3/Program/ScreenNames/ActiveName.dat \
  Manalink3/Program/ScreenNames/CirothUngol.scn
do
  expect_tracked_file "$path"
done
pass "tracked save/local-state inventory matches docs"

expect_file tools/list-security-scan-targets.sh
[ -x tools/list-security-scan-targets.sh ] || fail "tools/list-security-scan-targets.sh is not executable"
security_targets="$(tools/list-security-scan-targets.sh)"
security_header="$(printf '%s\n' "$security_targets" | sed -n '1p')"
[ "$security_header" = $'path\tkind\tbytes\tsha256' ] || fail "unexpected security target header: $security_header"
security_target_count="$(printf '%s\n' "$security_targets" | awk 'NR > 1 {count++} END {print count+0}')"
[ "$security_target_count" = "230" ] || fail "expected 230 tracked security scan targets, found $security_target_count"
for path in \
  Shandalar.exe \
  Program/Magic.exe \
  ManalinkEh.dll \
  Program/ManalinkEh.dll \
  Mods/Util/7za.exe \
  DeckInjector.jar \
  "Shandalar help.bat" \
  tools/create-cleanup-test-copy.sh \
  tools/create-git-handoff-bundle.sh \
  tools/print-manual-gameplay-baseline.sh \
  tools/print-security-scan-baseline.sh \
  tools/verify-handoff-readiness.sh \
  tools/verify-share-readiness.sh \
  tools/verify-crossover-mtg-state.sh \
  tools/list-security-scan-targets.sh
do
  printf '%s\n' "$security_targets" | awk -F '\t' -v p="$path" 'NR > 1 && $1 == p {found=1} END {exit found ? 0 : 1}' || fail "security scan target inventory is missing $path"
done
pass "security scan target inventory is generated ($security_target_count checked)"

for path in \
  README.md \
  AGENTS.md \
  docs/README.md \
  docs/branch-summary.md \
  docs/completion-audit.md \
  docs/git-handoff.md \
  docs/release-scope.md \
  docs/share-readiness.md \
  docs/distribution.md \
  docs/verified-on-this-machine.md \
  docs/runtime-manifest.md \
  docs/manual-gameplay-verification.md \
  docs/cleanup-audit.md \
  docs/cleanup-launch-copy-test.md \
  docs/duplicate-audit.md \
  docs/cleanup-move-plan.md \
  docs/save-state.md \
  docs/runtime-dependencies.md \
  docs/security-scan.md
do
  expect_file "$path"
done
pass "core docs exist"

python3 - <<'PY'
from pathlib import Path
import hashlib
import re
import sys

manifest = Path("docs/runtime-manifest.md")
row_re = re.compile(r"^\|\s*`([^`]+)`\s*\|\s*`([0-9a-f]{64})`\s*\|")
checked = 0
failures = []

for line in manifest.read_text(encoding="utf-8").splitlines():
    match = row_re.match(line)
    if not match:
        continue

    rel_path, expected = match.groups()
    path = Path(rel_path)
    checked += 1

    if not path.is_file():
        failures.append(f"missing manifest file: {rel_path}")
        continue

    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual != expected:
        failures.append(f"{rel_path} sha256 {actual} != {expected}")

if checked == 0:
    failures.append("runtime manifest contains no SHA-256 rows")

if failures:
    for failure in failures:
        print(f"manifest mismatch: {failure}", file=sys.stderr)
    sys.exit(1)

print(f"ok: runtime manifest hashes match current files ({checked} checked)")
PY

python3 - <<'PY'
from pathlib import Path
import sys

checked = 0
violations = []
allowed_suffixes = {".md", ".sh", ".vbs"}

for path in sorted(Path(".").rglob("*")):
    if ".git" in path.parts or path.parts[:2] == ("docs", "generated"):
        continue
    if not path.is_file() or path.suffix.lower() not in allowed_suffixes:
        continue

    checked += 1
    data = path.read_bytes()
    for line_no, line in enumerate(data.splitlines(), start=1):
        if any(byte > 0x7F for byte in line):
            violations.append((path, line_no))
            break

if violations:
    for path, line_no in violations:
        print(f"non-ASCII text: {path}:{line_no}", file=sys.stderr)
    sys.exit(1)

print(f"ok: maintained text files are ASCII ({checked} checked)")
PY

python3 - <<'PY'
from pathlib import Path
import sys

index_paths = [
    Path("README.md"),
    Path("docs/README.md"),
    Path("docs/share-readiness.md"),
    Path("docs/branch-summary.md"),
]
index_text = "\n".join(path.read_text(encoding="utf-8") for path in index_paths)

docs = [
    Path("AGENTS.md"),
    Path("archive/README.md"),
    Path("local/README.md"),
    Path("tools/README.md"),
]

for path in sorted(Path("docs").rglob("*.md")):
    if path.parts[:2] == ("docs", "generated") and path != Path("docs/generated/README.md"):
        continue
    docs.append(path)

missing = []
for path in docs:
    aliases = {path.as_posix()}
    if path.parts[0] == "docs":
        aliases.add(path.relative_to("docs").as_posix())

    if not any(alias in index_text for alias in aliases):
        missing.append(path.as_posix())

if missing:
    for path in missing:
        print(f"doc is not indexed: {path}", file=sys.stderr)
    sys.exit(1)

print(f"ok: maintained docs are indexed ({len(docs)} checked)")
PY

python3 - <<'PY'
from pathlib import Path
import re
import sys
import urllib.parse

root = Path(".")
link_re = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")
missing = []
checked = 0

for md in sorted(path for path in root.rglob("*.md") if ".git" not in path.parts):
    text = md.read_text(encoding="utf-8", errors="replace")
    for match in link_re.finditer(text):
        raw = match.group(1).strip()
        if not raw or raw.startswith(("#", "http://", "https://", "mailto:", "file:", "vscode:")):
            continue
        target = raw.split("#", 1)[0]
        if not target:
            continue
        if target.startswith("<") and target.endswith(">"):
            target = target[1:-1]
        target = urllib.parse.unquote(target)
        if re.match(r"^[A-Za-z]:[\\/]", target):
            continue

        checked += 1
        resolved = (md.parent / target).resolve()
        if not resolved.exists():
            line = text[:match.start()].count("\n") + 1
            missing.append((md, line, raw))

if missing:
    for md, line, raw in missing:
        print(f"missing markdown link: {md}:{line}: {raw}", file=sys.stderr)
    sys.exit(1)

print(f"ok: local Markdown links resolve ({checked} checked)")
PY

printf 'Share-readiness automated checks passed.\n'
