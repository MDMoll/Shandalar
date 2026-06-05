#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: tools/check-source-snapshot-parity.sh [--report-dir DIR] [--report-only]

Compare the top-level src/ source snapshot with Program/src/.

The full snapshot comparison is reported as evidence because these trees are
known to diverge. The command fails when exact-match files differ or when
source-safety markers from the current pass are missing from either snapshot.
It does not build, launch, copy, or modify runtime files.

Options:
  --report-dir DIR  Write source-snapshot-parity.tsv and
                    source-snapshot-parity-summary.txt under DIR.
  --report-only     Print failures but exit 0.
  -h, --help        Show this help.
USAGE
}

report_dir=""
report_only=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --report-dir)
      if [[ $# -lt 2 || -z "$2" ]]; then
        printf 'Missing value for --report-dir\n\n' >&2
        usage >&2
        exit 2
      fi
      report_dir="$2"
      shift 2
      ;;
    --report-only)
      report_only=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      printf 'Unknown option: %s\n\n' "$1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

exact_match_paths=(
  "card_id.h"
  "cards/draft.c"
  "cardartlib/src/main.cpp"
)

failures=0
warnings=0
same_count=0
different_count=0
missing_in_src_count=0
missing_in_program_count=0
total_count=0

emit() {
  local status="$1"
  local area="$2"
  local check="$3"
  local detail="$4"
  printf '%s\t%s\t%s\t%s\n' "$status" "$area" "$check" "$detail"
}

fail() {
  failures=$((failures + 1))
  emit "fail" "$@"
}

warn() {
  warnings=$((warnings + 1))
  emit "warn" "$@"
}

ok() {
  emit "ok" "$@"
}

sha256_file() {
  shasum -a 256 "$1" | awk '{print $1}'
}

is_exact_match_path() {
  local relpath="$1"
  local exact
  for exact in "${exact_match_paths[@]}"; do
    if [[ "$relpath" == "$exact" ]]; then
      return 0
    fi
  done
  return 1
}

is_marker_checked_path() {
  local relpath="$1"
  case "$relpath" in
    cards/avacyn_restored.c|cards/draft.c|functions/deck.c|functions/exiledby.c|functions/functions.c|functions/manipulate_and_damage_all.c|functions/show_backtrace.c|functions/targets.c|functions/produce_mana.c|functions/token_generation.c|functions/tutors.c|drawcardlib/config.c)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

src_list="$(mktemp "${TMPDIR:-/tmp}/shandalar-src-list.XXXXXX")"
program_list="$(mktemp "${TMPDIR:-/tmp}/shandalar-program-src-list.XXXXXX")"
all_list="$(mktemp "${TMPDIR:-/tmp}/shandalar-source-parity.XXXXXX")"
trap 'rm -f "$src_list" "$program_list" "$all_list"' EXIT

find src -type f -print | sed 's#^src/##' | sort > "$src_list"
find Program/src -type f -print | sed 's#^Program/src/##' | sort > "$program_list"
cat "$src_list" "$program_list" | sort -u > "$all_list"

parity_report=""
summary_report=""
if [[ -n "$report_dir" ]]; then
  mkdir -p "$report_dir"
  parity_report="$report_dir/source-snapshot-parity.tsv"
  summary_report="$report_dir/source-snapshot-parity-summary.txt"
  printf 'status\tpath\tsrc_sha256\tprogram_src_sha256\tnotes\n' > "$parity_report"
fi

while IFS= read -r relpath; do
  [[ -n "$relpath" ]] || continue

  total_count=$((total_count + 1))
  src_path="src/$relpath"
  program_path="Program/src/$relpath"
  src_hash=""
  program_hash=""
  notes=""

  if is_exact_match_path "$relpath"; then
    notes="exact-match-required"
  fi
  if is_marker_checked_path "$relpath"; then
    notes="${notes:+$notes;}marker-checked"
  fi

  if [[ -f "$src_path" && -f "$program_path" ]]; then
    src_hash="$(sha256_file "$src_path")"
    program_hash="$(sha256_file "$program_path")"
    if [[ "$src_hash" == "$program_hash" ]]; then
      status="same"
      same_count=$((same_count + 1))
    else
      status="different"
      different_count=$((different_count + 1))
    fi
  elif [[ -f "$src_path" ]]; then
    src_hash="$(sha256_file "$src_path")"
    status="missing-in-program"
    missing_in_program_count=$((missing_in_program_count + 1))
  elif [[ -f "$program_path" ]]; then
    program_hash="$(sha256_file "$program_path")"
    status="missing-in-src"
    missing_in_src_count=$((missing_in_src_count + 1))
  else
    status="missing-both"
    notes="${notes:+$notes;}unexpected"
  fi

  if [[ -n "$parity_report" ]]; then
    printf '%s\t%s\t%s\t%s\t%s\n' "$status" "$relpath" "$src_hash" "$program_hash" "$notes" >> "$parity_report"
  fi
done < "$all_list"

printf 'status\tarea\tcheck\tdetail\n'
ok "repo" "working-directory" "$repo_root"
ok "source-parity" "compared-relative-paths" "$total_count"

if [[ "$different_count" -gt 0 || "$missing_in_src_count" -gt 0 || "$missing_in_program_count" -gt 0 ]]; then
  warn "source-provenance" "src-vs-Program/src" "${different_count} different; ${missing_in_src_count} missing in src; ${missing_in_program_count} missing in Program/src"
else
  ok "source-provenance" "src-vs-Program/src" "all compared files match"
fi

for exact in "${exact_match_paths[@]}"; do
  if [[ -f "src/$exact" && -f "Program/src/$exact" ]] && cmp -s "src/$exact" "Program/src/$exact"; then
    ok "source-parity" "$exact" "exact-match source file matches"
  else
    fail "source-parity" "$exact" "exact-match source file differs or is missing"
  fi
done

check_marker() {
  local relpath="$1"
  local marker="$2"
  local label="$3"
  local missing=""
  local root

  for root in src Program/src; do
    if [[ ! -f "$root/$relpath" ]]; then
      missing="${missing:+$missing; }$root/$relpath missing"
    elif ! grep -Fq "$marker" "$root/$relpath"; then
      missing="${missing:+$missing; }$root/$relpath lacks marker"
    fi
  done

  if [[ -z "$missing" ]]; then
    ok "source-parity" "$relpath:$label" "marker present in both snapshots"
  else
    fail "source-parity" "$relpath:$label" "$missing"
  fi
}

check_marker "functions/targets.c" "static int read_auto_target_entries" "bounded-auto-target-helper"
check_marker "functions/targets.c" "while(count < 499 && fgets(buffer, sizeof(buffer), file))" "bounded-auto-target-read"
check_marker "functions/targets.c" "auto_targets[count] = -1;" "auto-target-sentinel"
check_marker "functions/targets.c" "static int bounded_targets_active_cards_count(int player)" "bounded-targets-active-count-helper"
check_marker "functions/targets.c" "static int bounded_targets_max_active_cards_count(void)" "bounded-targets-max-active-count-helper"
check_marker "functions/targets.c" "for(i=-1;i<active_count;i++){" "bounded-default-target-scan"
check_marker "functions/targets.c" "if (marked[p][c])" "bounded-marked-target-cleanup-scan"
check_marker "cards/draft.c" "static FILE* open_draft_output(const char* path, const char* mode)" "guarded-draft-output-helper"
check_marker "cards/draft.c" "FILE *file = open_draft_output(\"picks.txt\", \"w\");" "guarded-picks-log"
check_marker "cards/draft.c" "FILE *file2 = open_draft_output(\"packs.txt\", \"a+\");" "guarded-packs-log"
check_marker "cards/draft.c" "FILE *file = open_draft_output(buffer, \"w\");" "guarded-playdeck-output"
check_marker "cards/draft.c" "Invalid pick found: %d, pick=%d" "invalid-pick-log"
check_marker "cards/avacyn_restored.c" "td.allow_cancel = ! can_target(&td1) ? 0 : 1;" "descent-into-madness-hand-fallback"
check_marker "functions/ai.c" "int block_count = MIN(EXE_DWORD(0x607D54), 150);" "bounded-ai-blocker-count"
check_marker "functions/ai.c" "trigger_cause_controller = blocking_player;" "validated-ai-pay-to-block-trigger"
check_marker "functions/ai.c" "scnprintf(str, sizeof(str), \"%d: Entering AI Decision Phase.\\n\", EXE_DWORD(0x60EC40)++);" "bounded-ai-trace-prompt"
check_marker "functions/events.c" "static int bounded_active_cards_count(int player)" "bounded-events-active-count-helper"
check_marker "functions/events.c" "for (c = active_count; c < 150; ++c)" "bounded-events-processing-fill"
check_marker "functions/events.c" "scnprintf(buf, sizeof(buf), \"%d: Player #%d is processing %s(%d).\\n\"," "bounded-event-trace-prompt"
check_marker "functions/events.c" "scnprintf(buf, sizeof(buf), text_lines[0], opponent_name);" "bounded-event-proc-prompt"
check_marker "functions/engine.c" "scnprintf(buf, sizeof(buf), \"%d: Entering Untap Phase.\\n\", ++EXE_DWORD(0x60EC40));" "bounded-engine-untap-trace"
check_marker "functions/engine.c" "scnprintf(str, sizeof(str), \"%d: Entering UpKeep Phase.\\n\", EXE_DWORD(0x60EC40)++);" "bounded-engine-upkeep-trace"
check_marker "functions/engine.c" "static void append_to_mana_prompt(char** cursor, char* end, const char* fmt, ...)" "bounded-engine-mana-prompt-helper"
check_marker "functions/engine.c" "append_to_mana_prompt(&p, end, \"|%d\", generic);" "bounded-engine-generic-mana-prompt"
check_marker "functions/engine.c" "enum { GLOBAL_ALL_PURPOSE_BUFFER_SIZE = 1000 };" "bounded-engine-global-buffer-size"
check_marker "functions/engine.c" "scnprintf(p + written, GLOBAL_ALL_PURPOSE_BUFFER_SIZE - written, EXE_STR(0x786B00)" "bounded-engine-global-mana-prompt"
check_marker "functions/engine.c" "scnprintf(global_all_purpose_buffer_ptr(), GLOBAL_ALL_PURPOSE_BUFFER_SIZE, EXE_STR(0x786264)" "bounded-engine-activation-response-prompt"
check_marker "functions/engine.c" "scnprintf(global_all_purpose_buffer_ptr(), GLOBAL_ALL_PURPOSE_BUFFER_SIZE, EXE_STR(0x737bc0)" "bounded-engine-finalize-response-prompt"
check_marker "functions/engine.c" "static int bounded_engine_active_cards_count(int player)" "bounded-engine-active-count-helper"
check_marker "functions/engine.c" "int active_count = bounded_engine_active_cards_count(player);" "bounded-engine-player-scan"
check_marker "functions/engine.c" "for (c = 0; c < active_count; ++c, ++instance)" "bounded-engine-recalc-scan"
check_marker "functions/deck.c" "scnprintf(buf, sizeof(buf), \"get_card_instance(%d, %d)\", player, card);" "bounded-deck-instance-backtrace"
check_marker "functions/deck.c" "scnprintf(buf, sizeof(buf), \"%s\", text_lines[player + 1]);" "bounded-deck-draw-loss-prompt"
check_marker "functions/exiledby.c" "scnprintf(buf, sizeof(buf), \"No %s%scards have been exiled by %s.\"," "bounded-exiledby-empty-message"
check_marker "functions/exiledby.c" "scnprintf(buf, sizeof(buf), \"%s%scards exiled by %s\"," "bounded-exiledby-title"
check_marker "functions/exiledby.c" "static int bounded_exiledby_active_cards_count(int player)" "bounded-exiledby-active-count-helper"
check_marker "functions/exiledby.c" "int active_count = bounded_exiledby_active_cards_count(player);" "bounded-exiledby-legacy-scan"
check_marker "functions/functions.c" "int active_count = MIN(active_cards_count[i], 150);" "bounded-attack-power-scan"
check_marker "functions/functions.c" "for(k=0; k<active_count; k++){" "bounded-cip-removal-scan"
check_marker "functions/functions.c" "if (player < HUMAN || player > AI){" "count-subtype-player-guard"
check_marker "functions/functions.c" "int active_count = MIN(active_cards_count[player], 150);" "bounded-count-subtype-in-hand"
check_marker "functions/produce_mana.c" "for (count = 0; count < active_count && (color & 0x3F) != 0x3F; ++count){" "bounded-mana-color-scan"
check_marker "functions/produce_mana.c" "int active_count = MIN(active_cards_count[AI], 150);" "bounded-ai-mana-burn-scan"
check_marker "functions/produce_mana.c" "scnprintf(buf, sizeof(buf), \"%d mana left\", num_left);" "bounded-mana-choice-count-prompt"
check_marker "functions/produce_mana.c" "scnprintf(prompt, sizeof(prompt), \"%s (%s): What kind of mana?\"," "bounded-mana-flare-prompt"
check_marker "functions/multiblock.c" "static int bounded_active_cards_count(int player)" "bounded-multiblock-active-count-helper"
check_marker "functions/multiblock.c" "static int valid_card_slot(int card)" "bounded-multiblock-card-slot-helper"
check_marker "functions/multiblock.c" "int active_count = bounded_active_cards_count(player);" "bounded-multiblock-player-scan"
check_marker "functions/multiblock.c" "if (!valid_card_slot(directly_blocked))" "validated-multiblock-directly-blocked"
check_marker "functions/multiblock.c" "if (valid_card_slot(dsc) && in_play(opponent, dsc))" "validated-multiblocker-source-slot"
check_marker "functions/show_backtrace.c" "append_backtrace_text(char** cursor, char* end, const char* fmt, ...)" "bounded-backtrace-append-helper"
check_marker "functions/show_backtrace.c" "int wrote_dump = 0;" "guarded-backtrace-dump-open"
check_marker "functions/show_backtrace.c" "Could not write \\\"dump.dmp\\\" in your Manalink" "backtrace-dump-write-failure-message"
check_marker "functions/manipulate_and_damage_all.c" "static int bounded_active_cards_count(int player)" "bounded-manipulate-active-count-helper"
check_marker "functions/manipulate_and_damage_all.c" "int marked[2][150];" "expanded-manipulate-marked-slots"
check_marker "functions/manipulate_and_damage_all.c" "for (c = active_count - 1; c >= 0; --c)" "bounded-manipulate-reverse-scan"
check_marker "functions/manipulate_and_damage_all.c" "for( count = active_count-1; count >= 0; --count )" "bounded-damage-all-reverse-scan"
check_marker "functions/token_generation.c" "static int bounded_active_cards_count(int player)" "bounded-token-active-count-helper"
check_marker "functions/token_generation.c" "int active_count = bounded_active_cards_count(p);" "bounded-token-doubler-scan"
check_marker "functions/token_generation.c" "int c = bounded_active_cards_count(p)-1;" "bounded-legendary-token-scan"
check_marker "functions/tutors.c" "scnprintf(buf, sizeof(buf), \"%s:%d\\nillegal destination_chosen: %d with qty=%d\"" "bounded-tutor-illegal-choice-diagnostic"
check_marker "functions/tutors.c" "scnprintf(default_test.message, sizeof(default_test.message), \"Select up to %d cards.\", max_targets);" "bounded-tutor-graveyard-select-prompt"
check_marker "drawcardlib/config.c" "concat_cfg_key(const char* key1, const char* key2)" "allocated-config-key-helper"
check_marker "drawcardlib/config.c" "enum { PIC_HANDLE_NAME_BUFSIZE = 2 * MAX_PATH + 1200 };" "larger-diagnostic-buffer"
check_marker "drawcardlib/config.c" "snprintf(buf[idx], sizeof(buf[idx])," "bounded-frame-diagnostic"
check_marker "drawcardlib/config.c" "char* key = concat_cfg_key(key1, key2);" "allocated-get-cfg-int-key"
check_marker "drawcardlib/config.c" "char* key = concat_cfg_key(\"font\", font_name);" "allocated-font-key"
check_marker "functions/functions.c" "if( (mode & (GAA_DAMAGE_PREVENTION | GAA_DAMAGE_PREVENTION_PLAYER | GAA_DAMAGE_PREVENTION_CREATURE | GAA_DAMAGE_PREVENTION_ME))" "generic-activated-damage-prevention-window"

emit "summary" "result" "same" "$same_count"
emit "summary" "result" "different" "$different_count"
emit "summary" "result" "missing-in-src" "$missing_in_src_count"
emit "summary" "result" "missing-in-program" "$missing_in_program_count"
emit "summary" "result" "failures" "$failures"
emit "summary" "result" "warnings" "$warnings"

if [[ -n "$summary_report" ]]; then
  {
    printf 'Source Snapshot Parity Summary\n'
    printf 'repo_root: %s\n' "$repo_root"
    printf 'total_relative_paths: %s\n' "$total_count"
    printf 'same: %s\n' "$same_count"
    printf 'different: %s\n' "$different_count"
    printf 'missing_in_src: %s\n' "$missing_in_src_count"
    printf 'missing_in_program: %s\n' "$missing_in_program_count"
    printf 'exact_match_files: %s\n' "${#exact_match_paths[@]}"
    printf 'parity_failures: %s\n' "$failures"
    printf 'warnings: %s\n' "$warnings"
    printf 'report: %s\n' "$parity_report"
  } > "$summary_report"
  ok "source-parity" "report" "$parity_report"
  ok "source-parity" "summary" "$summary_report"
fi

if [[ "$failures" -gt 0 && "$report_only" -eq 0 ]]; then
  exit 1
fi

exit 0
