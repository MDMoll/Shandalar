#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
install_root="${1:-$repo_root}"

usage() {
  cat <<'EOF'
Usage: tools/verify-loam-larva-basic-land-picker.sh [install-root]

Verifies the static evidence for the Loam Larva adventure-duel basic-land
library picker fix. It checks the patched Shandalar.dll hash/bytes in an
install-root-shaped tree, then checks the repo source/data assumptions that the
binary cave depends on.

This does not launch the game or prove the deck picker renders. A visible
Shandalar adventure-duel Loam Larva retest is still required for gameplay proof.

If install-root is omitted, the repository root is checked.
EOF
}

if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
  usage
  exit 0
fi

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

expect_hash() {
  local path="$1"
  local expected="$2"
  expect_file "$path"
  local actual
  actual="$(shasum -a 256 "$path" | awk '{print $1}')"
  [ "$actual" = "$expected" ] || fail "$path sha256 $actual != $expected"
}

expect_hex() {
  local path="$1"
  local offset="$2"
  local length="$3"
  local expected="$4"
  expect_file "$path"
  local actual
  actual="$(xxd -p -l "$length" -s "$offset" "$path" | tr -d '\n')"
  [ "$actual" = "$expected" ] || fail "$path hex at $offset $actual != $expected"
}

[ -d "$install_root" ] || fail "install root is not a directory: $install_root"
install_root="$(cd "$install_root" && pwd)"

shandalar_dll_hash=3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41
loam_hook_hex=e9c6e1cd00
loam_cave_hex=807e290d0f84f11c32ffbe01000000c745f001000000e9951c32ff

for rel_path in Shandalar.dll Program/Shandalar.dll; do
  path="$install_root/$rel_path"
  expect_hash "$path" "$shandalar_dll_hash"
  expect_hex "$path" 0x7e395 5 "$loam_hook_hex"
  expect_hex "$path" 0x1174960 27 "$loam_cave_hex"
done
pass "Shandalar.dll Loam Larva basic-land selector patch bytes match in $install_root"

export SHANDALAR_REPO_ROOT="$repo_root"
python3 - <<'PY'
from pathlib import Path
import csv
import os
import re
import sys

repo_root = Path(os.environ["SHANDALAR_REPO_ROOT"])


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    sys.exit(1)


def ok(message: str) -> None:
    print(f"ok: {message}")


def text(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def expect_regex(path: Path, pattern: str, description: str) -> None:
    if not re.search(pattern, text(path), flags=re.MULTILINE):
        fail(f"{path} missing {description}")


for rel in ("src/defs.h", "Program/src/defs.h"):
    path = repo_root / rel
    expect_regex(path, r"\bSUB_BASIC_LAND\s*=\s*13\b", "SUB_BASIC_LAND = 13")
    expect_regex(path, r"uint8_t\s+type;\s*//.*\n\s*uint8_t\s+subtype;", "packed type/subtype adjacency")

for rel in ("src/subtypes.h", "Program/src/subtypes.h"):
    path = repo_root / rel
    expect_regex(path, r"\bSUBTYPE_BASIC\s*=\s*0x2000\b", "SUBTYPE_BASIC = 0x2000")
    expect_regex(path, r"\bHARDCODED_SUBTYPE_BASIC_LAND\s*=\s*188\b", "hardcoded basic-land subtype id")

for rel in ("src/cards/oath_of_the_gatewatch.c",):
    body = text(repo_root / rel)
    for needle in (
        "int card_loam_larva",
        '"Select a basic land card."',
        "test.subtype = SUBTYPE_BASIC;",
        "new_global_tutor",
    ):
        if needle not in body:
            fail(f"{rel} missing {needle}")

csv_path = repo_root / "magic_updater" / "Manalink.csv"
if not csv_path.is_file():
    fail(f"missing {csv_path}")

rows_by_name = {}
with csv_path.open(encoding="utf-8", errors="replace", newline="") as handle:
    for row in csv.reader(handle, delimiter=";"):
        if len(row) > 34 and row[1]:
            rows_by_name[row[1]] = row

loam = rows_by_name.get("Loam Larva")
if loam is None:
    fail("magic_updater/Manalink.csv missing Loam Larva")
loam_text = " ".join(loam).lower()
for phrase in ("basic land card", "top of it"):
    if phrase not in loam_text:
        fail(f"Loam Larva CSV row missing phrase {phrase!r}")

for name in ("Forest", "Island", "Mountain", "Plains", "Swamp"):
    row = rows_by_name.get(name)
    if row is None:
        fail(f"magic_updater/Manalink.csv missing {name}")
    type_line = row[34].lower()
    if "basic land" not in type_line or name.lower() not in type_line:
        fail(f"{name} CSV row does not advertise a basic-land type line: {row[34]!r}")
    if row[24].lower() != "2000h":
        fail(f"{name} CSV row first subtype {row[24]!r} is not SUBTYPE_BASIC (2000h)")
    if row[25].lower() != "1004h":
        fail(f"{name} CSV row second subtype {row[25]!r} is not SUBTYPE_LAND (1004h)")

ok("Loam Larva source/data guardrails match the Shandalar.dll patch assumptions")
PY

cat <<'EOF'
ok: static Loam Larva basic-land picker verification passed
note: this is not visible gameplay proof; restart Shandalar and retest Loam Larva in an adventure duel to close the manual boundary.
EOF
