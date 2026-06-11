#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: tools/verify-deck-injector.sh [jar-path]

Verifies the Deck Injector jar was built for Java 21 and that Scrubland maps to
duel id 216 / Shandalar game code 0900.

Environment overrides:
  JAVA=/path/to/java
  JAVAC=/path/to/javac
  JAVAP=/path/to/javap
EOF
}

if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
  usage
  exit 0
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
jar_path="${1:-$repo_root/DeckInjector.jar}"
if [[ "$jar_path" != /* ]]; then
  jar_path="$repo_root/$jar_path"
fi

java_bin="${JAVA:-java}"
javac_bin="${JAVAC:-javac}"
javap_bin="${JAVAP:-javap}"

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

require_tool() {
  local tool="$1"
  command -v "$tool" >/dev/null 2>&1 || fail "$tool was not found"
}

require_tool "$java_bin"
require_tool "$javac_bin"
require_tool "$javap_bin"
require_tool unzip

[ -f "$jar_path" ] || fail "missing jar: $jar_path"

main_class="$(unzip -p "$jar_path" META-INF/MANIFEST.MF | tr -d '\r' | awk -F': ' '$1 == "Main-Class" {print $2; exit}')"
[ "$main_class" = "gui.tabbedGui" ] || fail "unexpected Main-Class: ${main_class:-<missing>}"
pass "Deck Injector main class is gui.tabbedGui"

major="$("$javap_bin" -verbose -classpath "$jar_path" gui.tabbedGui | awk '/major version:/ {print $3; exit}')"
[ "$major" = "65" ] || fail "expected Java 21 classfile major 65, found ${major:-<missing>}"
pass "Deck Injector classfile major is 65"

allcards_constants="$("$javap_bin" -verbose -classpath "$jar_path" gui.allCards)"
grep -q 'Scrubland' <<< "$allcards_constants" || fail "compiled allCards does not contain Scrubland"
! grep -q 'Scubland' <<< "$allcards_constants" || fail "compiled allCards still contains Scubland"
pass "Scrubland spelling is present and Scubland is absent in compiled allCards"

tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/deck-injector-verify.XXXXXX")"
cleanup() {
  rm -rf "$tmpdir"
}
trap cleanup EXIT

cat > "$tmpdir/VerifyDeckInjector.java" <<'EOF'
import gui.allCards;

public final class VerifyDeckInjector {
    public static void main(String[] args) {
        allCards cards = new allCards();
        require(cards.containsKey("Scrubland"), "Scrubland key missing");
        require(!cards.containsKey("Scubland"), "Scubland typo key still present");
        require(cards.gameToDuel("0900") == 216, "game code 0900 did not map to duel id 216");
        require("0900".equals(cards.duelToGame(216)), "duel id 216 did not map to game code 0900");
        require("Scrubland".equals(cards.gameToName("0900")), "game code 0900 did not map to Scrubland");
        require("Scrubland".equals(cards.duelToName(216)), "duel id 216 did not map to Scrubland");
    }

    private static void require(boolean condition, String message) {
        if (!condition) {
            throw new AssertionError(message);
        }
    }
}
EOF

"$javac_bin" --release 21 -cp "$jar_path" -d "$tmpdir" "$tmpdir/VerifyDeckInjector.java"
"$java_bin" -Djava.awt.headless=true -cp "$tmpdir:$jar_path" VerifyDeckInjector
pass "Scrubland mapping probe passed"
