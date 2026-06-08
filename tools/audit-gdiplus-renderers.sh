#!/usr/bin/env bash
set -euo pipefail

repo_root="${1:-.}"
cd "$repo_root"

failures=0
warnings=0

ok() {
  printf 'ok: %s\n' "$1"
}

warn() {
  warnings=$((warnings + 1))
  printf 'warning: %s\n' "$1"
}

fail() {
  failures=$((failures + 1))
  printf 'FAIL: %s\n' "$1"
}

require_file() {
  local path="$1"
  if [[ -f "$path" ]]; then
    ok "found $path"
  else
    fail "missing $path"
  fi
}

require_marker() {
  local path="$1"
  local marker="$2"
  local label="$3"

  if rg -qF "$marker" "$path"; then
    ok "$label"
  else
    fail "$label missing marker: $marker"
  fi
}

assert_no_gdiplus_startup_in_dllmain() {
  local path="$1"

  if perl -0ne '
    while (/DllMain\s*\([^)]*\)\s*\{(.*?)\n\}/sg) {
      exit 1 if $1 =~ /GdiplusStartup\s*\(/;
    }
  ' "$path"; then
    ok "$path does not start GDI+ in DllMain"
  else
    fail "$path starts GDI+ in DllMain"
  fi
}

check_pe_gdiplus_dll() {
  local path="$1"
  require_file "$path"
  [[ -f "$path" ]] || return

  local dump
  dump="$(objdump -p "$path" 2>/dev/null || true)"
  if printf '%s\n' "$dump" | rg -qi 'DLL Name: .*gdiplus\.dll'; then
    ok "$path imports gdiplus.dll"
  else
    fail "$path does not import gdiplus.dll"
  fi

  local characteristics
  characteristics="$(printf '%s\n' "$dump" | awk '/DllCharacteristics/{print $2; exit}')"
  if [[ "$characteristics" == "00000000" ]]; then
    ok "$path keeps legacy-safe DllCharacteristics=00000000"
  else
    fail "$path has DllCharacteristics=${characteristics:-missing}"
  fi
}

check_drawcardlib_source() {
  local path="$1"
  require_file "$path"
  [[ -f "$path" ]] || return

  require_marker "$path" "input.SuppressBackgroundThread = 1;" "$path suppresses GDI+ background thread"
  require_marker "$path" "input.SuppressExternalCodecs = 1;" "$path suppresses external GDI+ codecs"
  require_marker "$path" "gdiplus_startup_output.NotificationHook(&gdiplus_bg_thread_token);" "$path calls GDI+ notification hook"
  require_marker "$path" "if (stat != Ok)" "$path checks GDI+ notification hook status"
  require_marker "$path" "gdiplus_startup_output.NotificationUnhook(gdiplus_bg_thread_token);" "$path calls GDI+ notification unhook"
  require_marker "$path" "gdip_create_graphics(HDC hdc" "$path has checked graphics wrapper"
  require_marker "$path" "gdip_create_bitmap_from_scan0(INT width" "$path has checked bitmap wrapper"
  require_marker "$path" "gdip_draw_image_rect_rect(GpGraphics* graphics" "$path has checked rect-rect draw wrapper"
  require_marker "$path" "gdip_lock_bits(GpBitmap* bitmap" "$path has checked bitmap lock wrapper"
  assert_no_gdiplus_startup_in_dllmain "$path"
}

check_cardartlib_source() {
  local path="$1"
  require_file "$path"
  [[ -f "$path" ]] || return

  require_marker "$path" "ensure_gdiplus_started(void)" "$path starts GDI+ lazily"
  require_marker "$path" "gdiplusStartupInput.SuppressBackgroundThread = TRUE;" "$path suppresses GDI+ background thread"
  require_marker "$path" "gdiplusStartupInput.SuppressExternalCodecs = TRUE;" "$path suppresses external GDI+ codecs"
  require_marker "$path" "tmpSurface->GetLastStatus() != gdi::Ok" "$path validates image load status"
  require_marker "$path" "if (stat != gdi::Ok)" "$path checks GDI+ notification hook status"
  require_marker "$path" "gdiplusStartupOutput.NotificationUnhook(gdiplusBGThreadToken);" "$path calls GDI+ notification unhook"
  assert_no_gdiplus_startup_in_dllmain "$path"
}

summarize_gdip_calls() {
  local path="$1"
  [[ -d "$path" ]] || return

  local count
  count="$({ rg -n 'Gdip[A-Za-z0-9_]+\(' "$path" --glob '*.[ch]' --glob '*.cpp' --glob '!gdiplus.h' --glob '!drawcardlib.h' --glob '!drawcardlib.c' 2>/dev/null || true; } | { rg -v 'GdipDisposeImage(Attributes)?\(' || true; } | wc -l | tr -d ' ')"
  printf 'info: %s has %s direct non-wrapper GDI+ API callsite(s)\n' "$path" "$count"
  if [[ "$path" == *drawcardlib && "$count" -ne 0 ]]; then
    fail "$path has direct non-wrapper GDI+ callsites"
  fi
}

check_drawcardlib_source src/drawcardlib/drawcardlib.c
check_drawcardlib_source Program/src/drawcardlib/drawcardlib.c
check_cardartlib_source src/cardartlib/src/main.cpp
check_cardartlib_source Program/src/cardartlib/src/main.cpp

check_pe_gdiplus_dll Drawcardlib.dll
check_pe_gdiplus_dll Program/Drawcardlib.dll
check_pe_gdiplus_dll CardArtLib.dll
check_pe_gdiplus_dll Program/CardArtLib.dll

summarize_gdip_calls src/drawcardlib
summarize_gdip_calls Program/src/drawcardlib
summarize_gdip_calls src/cardartlib
summarize_gdip_calls Program/src/cardartlib

if [[ "$failures" -ne 0 ]]; then
  printf 'GDI+ renderer audit failed with %s failure(s) and %s warning(s).\n' "$failures" "$warnings"
  exit 1
fi

printf 'GDI+ renderer audit passed with %s warning(s).\n' "$warnings"
