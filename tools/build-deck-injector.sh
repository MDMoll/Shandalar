#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: tools/build-deck-injector.sh [output-jar]

Builds DeckInjector.jar from src/gui with JDK 21 bytecode.

Environment overrides:
  JAVAC=/path/to/javac
  JAR=/path/to/jar
EOF
}

if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
  usage
  exit 0
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output="${1:-$repo_root/DeckInjector.jar}"
if [[ "$output" != /* ]]; then
  output="$repo_root/$output"
fi

javac_bin="${JAVAC:-javac}"
jar_bin="${JAR:-jar}"

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

require_major_at_least_21() {
  local tool="$1"
  local version_arg="$2"
  local version_output version
  command -v "$tool" >/dev/null 2>&1 || fail "$tool was not found"
  version_output="$("$tool" "$version_arg" 2>&1 | sed -n '1p')"
  version="$(printf '%s\n' "$version_output" | sed -E 's/^[^0-9]*([0-9]+)(\..*)?$/\1/')"
  [ -n "$version" ] || fail "could not parse $tool version from: $version_output"
  [ "$version" -ge 21 ] || fail "$tool must be JDK 21 or newer; found: $version_output"
}

require_major_at_least_21 "$javac_bin" -version
require_major_at_least_21 "$jar_bin" --version

tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/deck-injector-build.XXXXXX")"
cleanup() {
  rm -rf "$tmpdir"
}
trap cleanup EXIT

classes_dir="$tmpdir/classes"
manifest="$tmpdir/MANIFEST.MF"
tmpjar="$tmpdir/DeckInjector.jar"
mkdir -p "$classes_dir"

cat > "$manifest" <<'EOF'
Manifest-Version: 1.0
Main-Class: gui.tabbedGui
Created-By: JDK 21 Deck Injector build script

EOF

cd "$repo_root"
"$javac_bin" --release 21 -encoding UTF-8 -d "$classes_dir" src/gui/*.java
"$jar_bin" --create --file "$tmpjar" --manifest "$manifest" --date=2026-06-08T00:00:00Z -C "$classes_dir" .
mv "$tmpjar" "$output"

printf 'Built %s\n' "$output"
shasum -a 256 "$output"
