// -*- tab-width:8; c-basic-offset:2; -*-

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "deckdll.h"
#include "File.h"

static const char*
safe_str(const char* str, const char* fallback)
{
  return str ? str : fallback;
}

static const char*
file_errstr(int en)
{
  return en ? strerror(en) : "unknown error";
}

static unsigned long
ulong_size(size_t value)
{
  return static_cast<unsigned long>(value);
}

// caller assumes responsibility for keeping filename live
File::File(const char* filename, const char* mode, bool no_die_on_failed_open)
{
  this->filename_ = safe_str(filename, "<null filename>");
  this->f_ = NULL;

  if (!filename || !mode)
    {
      if (!no_die_on_failed_open)
	fatal("Couldn't open \"%s\": %s", this->filename_, !filename ? "missing filename" : "missing mode");
      return;
    }

  errno = 0;
  this->f_ = fopen(filename, mode);
  if (!this->f_ && !no_die_on_failed_open)
    fatal("Couldn't open \"%s\": %s", this->filename_, file_errstr(errno));
}

File::~File(void)
{
  if (this->f_)
    {
      errno = 0;
      if (fclose(this->f_))
	fatal("Couldn't close \"%s\": %s", this->filename_, file_errstr(errno));
      this->f_ = NULL;
    }
}

void
File::close(void)
{
  if (this->f_)
    {
      errno = 0;
      if (fclose(this->f_))
	{
	  fatal("Couldn't close \"%s\": %s", this->filename_, file_errstr(errno));
	  return;
	}
    }

  this->f_ = NULL;
}

void
File::seek(long offset, int whence)
{
  if (!this->f_)
    {
      fatal("Couldn't seek in \"%s\": file already closed", this->filename_);
      return;
    }

  errno = 0;
  if (fseek(this->f_, offset, whence))
    fatal("Couldn't seek in \"%s\": %s", this->filename_, file_errstr(errno));
}

void
File::read(void* dest, size_t count, const char* dest_name)
{
  if (!this->f_)
    {
      fatal("Couldn't read from \"%s\": file already closed", this->filename_);
      return;
    }

  if (!dest && count)
    {
      fatal("Couldn't read from \"%s\": null destination for %s", this->filename_, safe_str(dest_name, "<unnamed buffer>"));
      return;
    }

  if (!count)
    return;

  errno = 0;
  clearerr(this->f_);

  size_t bytes_read = fread(dest, 1, count, this->f_);
  if (bytes_read != count)
    {
      int en = errno;
      const char* eof = feof(this->f_) ? " (EOF)" : "";
      fatal("Read only %lu bytes of %s; expected %lu: %s%s",
	    ulong_size(bytes_read),
	    safe_str(dest_name, "<unnamed buffer>"),
	    ulong_size(count),
	    file_errstr(en),
	    eof);
    }
}

void
File::fgets(char* dest, int sz)
{
  if (!this->f_)
    {
      fatal("Couldn't read from \"%s\": file already closed", this->filename_);
      return;
    }

  if (!dest || sz <= 0)
    {
      fatal("Couldn't read from \"%s\": invalid line buffer", this->filename_);
      return;
    }

  dest[0] = 0;

  if (sz == 1)
    return;

  errno = 0;
  clearerr(this->f_);

  if (!::fgets(dest, sz, this->f_))
    {
      if (feof(this->f_))
	*dest = 0;
      else
	{
	  int en = errno;
	  fatal("Couldn't read from \"%s\": %s", this->filename_, file_errstr(en));
	}
    }
}

char*
File::readline(char* dest, int sz)
{
  if (!dest || sz <= 0)
    {
      fatal("Couldn't read line from \"%s\": invalid line buffer", this->filename_);
      return NULL;
    }

  dest[0] = 0;
  if (sz > 1)
    dest[sz - 1] = 0;

  this->fgets(dest, sz);

  if (!dest[0])	// eof or other error
    return NULL;

  size_t len = strlen(dest);

  if (len && dest[len - 1] == '\n')
    {
      dest[--len] = 0;
      if (len && dest[len - 1] == '\r')
	dest[len - 1] = 0;
      return dest;
    }

  if (len && dest[len - 1] == '\r')
    {
      dest[len - 1] = 0;
      return dest;
    }

  // Line in file too long; read until eol or EOF.
  char rest_of_line[512];
  do
    {
      rest_of_line[0] = 0;
      this->fgets(rest_of_line, sizeof(rest_of_line));
    } while (rest_of_line[0] && !strchr(rest_of_line, '\n'));

  return dest;
}

void
File::write(void* src, size_t count, const char* src_name)
{
  if (!this->f_)
    {
      fatal("Couldn't write to \"%s\": file already closed", this->filename_);
      return;
    }

  if (!src && count)
    {
      fatal("Couldn't write to \"%s\": null source for %s", this->filename_, safe_str(src_name, "<unnamed buffer>"));
      return;
    }

  if (!count)
    return;

  errno = 0;
  clearerr(this->f_);

  size_t bytes_written = fwrite(src, 1, count, this->f_);
  if (bytes_written != count)
    {
      int en = errno;
      const char* eof = feof(this->f_) ? " (EOF)" : "";
      fatal("Wrote only %lu bytes of %s; expected %lu: %s%s",
	    ulong_size(bytes_written),
	    safe_str(src_name, "<unnamed buffer>"),
	    ulong_size(count),
	    file_errstr(en),
	    eof);
    }
}

void
File::printf(const char* fmt, ...)
{
  if (!this->f_)
    {
      fatal("Couldn't write to \"%s\": file already closed", this->filename_);
      return;
    }

  if (!fmt)
    {
      fatal("Couldn't write to \"%s\": null format string", this->filename_);
      return;
    }

  errno = 0;

  va_list args;
  va_start(args, fmt);
  int rval = vfprintf(this->f_, fmt, args);
  int en = errno;
  va_end(args);

  if (rval < 0)
    fatal("Couldn't write to \"%s\": %s", this->filename_, file_errstr(en));
}

void
File::flush(void)
{
  if (!this->f_)
    {
      fatal("Couldn't flush \"%s\": file already closed", this->filename_);
      return;
    }

  errno = 0;
  if (fflush(this->f_))
    fatal("Couldn't flush \"%s\": %s", this->filename_, file_errstr(errno));
}