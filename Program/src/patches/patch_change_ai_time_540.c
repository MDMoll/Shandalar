// -*- tab-width:8 -*-
#include "patch.h"

// Historical helper for unhooked Magic.exe builds.
//
// The active runtime moved check_timer_for_ai_speculation() into C with
// patch_change_ai_time.pl. For current binaries, patch the ManalinkEh.dll
// fallback with tools/patch-ai-decision-fallback.py instead.
//
// The executable name is preserved for old build scripts.

int
main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  FILE* f;

  OPEN("Magic.exe");

  /*************************************************************************************
  * Replace constant in check_timer_for_ai_speculation() at 0x472d10 from 5405 to 270. *
  *************************************************************************************/
  /*
   * Previous contents:
   * 472d10:	e8 7b ff ff ff		# call	0x472c90		; eax = get_usertime_of_current_thread_in_ms()
   * 472d15:	2b 05 64 bf 56 00	# sub	eax, dword [0x56bf64]	; eax -= start_usertime_of_current_thread_in_ms
   * 472d1b:	c1 e0 02		# shl	eax, 0x2		; eax *= 4;
   * 472d1e:	8d 04 80		# lea	eax, [eax+eax*4]	; eax *= 5;
   * 472d21:	8d 04 80		# lea	eax, [eax+eax*4]	; eax *= 5;
   * 472d24:	b9 1d 15 00 00		# mov	ecx, 0x151d		; ecx = 5405
   * 472d29:	99			# cdq				; sign-extend eax into edx
   * 472d2a:	f7 f9			# idiv	ecx			; eax = edx:eax / ecx; edx = edx:eax % ecx
   * 472d2c:	c3			# ret				; return
   */

  {
    const char legacy_preimage[] = "\xb9\x1d\x15\x00\x00";
    const char hooked_preimage[] = "\x90\x90\x90\x90\x90";
    char actual[5];

    if (fseek(f, 0x72d24, SEEK_SET))
      die("Couldn't seek to %x in %s", 0x72d24, filename);

    if (fread(actual, 1, sizeof(actual), f) != sizeof(actual))
      die("Couldn't read preimage at %x in %s", 0x72d24, filename);

    if (memcmp(actual, legacy_preimage, sizeof(actual)) != 0) {
      if (memcmp(actual, hooked_preimage, sizeof(actual)) == 0)
	popup("patch_change_ai_time_540",
	      "Skipped Magic.exe: the active check_timer_for_ai_speculation() path already jumps into C. Patch ManalinkEh.dll with tools/patch-ai-decision-fallback.py instead.");
      else
	popup("patch_change_ai_time_540",
	      "Skipped Magic.exe: unexpected bytes at 0x72d24:%s",
	      buf_to_hexstr(actual, sizeof(actual)));
      CLOSE();
      return 1;
    }
  }

#define INJ	"\xb9\x0e\x01\x00\x00"	/* mov	ecx, 0x010e		; ecx = 270	*/
  SEEK_AND_WRITE(f, 0x72d24);
#undef INJ

  CLOSE();

  SUCCESS("patch_change_ai_time_540");
}
