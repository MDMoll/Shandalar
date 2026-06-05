// -*- c-basic-offset: 2 -*-
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

USHORT WINAPI RtlCaptureStackBackTrace(ULONG FramesToSkip, ULONG FramesToCapture, PVOID *BackTrace, PULONG BackTraceHash);

void popup(const char* title, const char* fmt, ...);

static void
append_backtrace_text(char** cursor, char* end, const char* fmt, ...)
{
  if (*cursor >= end)
	return;

  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(*cursor, end - *cursor, fmt, args);
  va_end(args);

  if (written < 0)
	return;
  if (written >= end - *cursor)
	*cursor = end - 1;
  else
	*cursor += written;
}

void
show_backtrace(const char* title, const char* header)
{
  PVOID frames[100];

  int numframes = RtlCaptureStackBackTrace(0, 30, &frames[0], NULL);

  char buf[4000];
  char* p = &buf[0];
  char* end = buf + sizeof(buf);
  *p = 0;
  append_backtrace_text(&p, end, "%s\n", header);
  int i;
  for (i = 0; i < numframes; ++i)
	append_backtrace_text(&p, end, "%d: 0x%p\n", i, frames[i]);

  int wrote_dump = 0;
  FILE* f = fopen("dump.dmp", "w");
  if (f)
	{
	  fprintf(f, "%s\n%s", title, buf);
	  fclose(f);
	  wrote_dump = 1;
	}

  if (wrote_dump)
	append_backtrace_text(&p, end, "\n"
						  "This information has been included in a file\n"
						  "\"dump.dmp\" in your Manalink program directory.\n"
						  "Please include it (NOT a screenshot) if you\n"
						  "report this bug.");
  else
	append_backtrace_text(&p, end, "\n"
						  "Could not write \"dump.dmp\" in your Manalink\n"
						  "program directory. Please include this message\n"
						  "if you report this bug.");

  popup(title, buf);
}

void
backtrace_and_abort(const char* header)
{
  show_backtrace("Assertion failed", header);
  abort();
}
