MINGW_PREFIX ?= $(shell if command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then printf 'i686-w64-mingw32-'; elif command -v i386-mingw32-gcc >/dev/null 2>&1; then printf 'i386-mingw32-'; fi)
CC = $(MINGW_PREFIX)gcc
CFLAGS = -ggdb2 -O2 -m32 -fno-omit-frame-pointer -march=i586		\
	-mpreferred-stack-boundary=2 -Wall -Wextra -Wpointer-arith	\
	-Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings	\
	-Wshadow -W -Wstrict-prototypes -Wnested-externs -Winline	\
	-Werror -I.. -I.
LDFLAGS =
COMPILE = $(CC) $(CFLAGS) -c
RM = rm -f

HFILES := $(wildcard *.h)
TGTS = $(patsubst %.c,%.exe,$(wildcard *.c))
INSTALLDIR = ../..

all: testwhichgcc $(TGTS)

# A trap to keep me from accidentally compiling with host or Cygwin gcc.
testwhichgcc:
	@target="$$( $(CC) -dumpmachine 2>/dev/null )"; \
	case "$$target" in *mingw32*|*mingw64*) true ;; \
	  *) echo "$(CC) targets $$target instead of MinGW"; false ;; \
	esac

install: testwhichgcc $(patsubst %,$(INSTALLDIR)/%,$(TGTS))

.PHONY: testwhichgcc clean

%.exe: %.c patch.h
	$(CC) $(CFLAGS) -static -static-libgcc -mwindows -s -o $@ $< $(LDFLAGS)

$(INSTALLDIR)/%: %
	/bin/cp $< $@

clean:
	$(RM) $(TGTS)
