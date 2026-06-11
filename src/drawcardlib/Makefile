MINGW_PREFIX ?= $(shell if command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then printf 'i686-w64-mingw32-'; elif command -v i386-mingw32-gcc >/dev/null 2>&1; then printf 'i386-mingw32-'; fi)
CC = $(MINGW_PREFIX)gcc
ASM = yasm
OBJCOPY = $(MINGW_PREFIX)objcopy
DLLTOOL = $(MINGW_PREFIX)dlltool
CP = /bin/cp -f
RM = rm -f

CPPFLAGS = -I.. -I.
WARNINGS_AS_ERRORS ?= 0
WERROR ?=
ifeq ($(WARNINGS_AS_ERRORS),1)
WERROR := -Werror
endif
CFLAGS = -ggdb2 -O2 -m32 -fno-omit-frame-pointer -march=i586		\
	-mpreferred-stack-boundary=2 -Wall -Wextra -Wpointer-arith	\
	-Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings	\
	-Wshadow -W -Wstrict-prototypes -Wnested-externs -Winline	\
	$(WERROR)
DEPFLAGS = -MMD -MP
AFLAGS = -a x86 -m x86 -p nasm -r nasm -f win32 -I.. -I.

EXTERNDLLS = image.dll Cardartlib.dll
LDFLAGS = -Wl,--enable-stdcall-fixup -Wl,--defsym,___ImageBase=__image_base__ \
	-Wl,--disable-dynamicbase -Wl,--disable-nxcompat \
	-L. $(patsubst %.dll,-l%,$(EXTERNDLLS)) -lgdi32 -lgdiplus -lshlwapi -lMsimg32

COMPILE = $(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c

HFILES := $(sort $(wildcard *.h))
CFILES := $(sort $(wildcard *.c))
AFILES := $(filter-out DrawCard.asm,$(sort $(wildcard *.asm)))
TESTAFILES := $(sort $(wildcard t/*.asm))

COBJFILES := $(patsubst %.c,%.o,$(CFILES))
AOBJFILES := $(patsubst %.asm,%.ao,$(AFILES))
TESTAOBJFILES := $(patsubst %.asm,%.ao,$(TESTAFILES))
DEPFILES := $(COBJFILES:.o=.d)

LIBFILES = $(patsubst %.dll,lib%.a,$(EXTERNDLLS))
TGTDLL = Drawcardlib.dll
TGTDBG = $(patsubst %.dll,%.dbg,$(TGTDLL))
DEFFILE = $(patsubst %.dll,%.def,$(TGTDLL))
INSTALLDIR = ../..

.PHONY: all test testwhichgcc clean install

all: testwhichgcc $(TGTDLL)

test: test.exe
	./test.exe

test.exe: $(LIBFILES) $(COBJFILES) $(AOBJFILES) $(TESTAOBJFILES)
	$(CC) -m32 -o $@ $(COBJFILES) $(AOBJFILES) $(TESTAOBJFILES) $(LDFLAGS) -lwinmm

install: testwhichgcc $(INSTALLDIR)/$(TGTDLL) $(INSTALLDIR)/$(TGTDBG)

$(INSTALLDIR)/$(TGTDLL): $(TGTDLL)
	$(CP) $(TGTDLL) $(INSTALLDIR)/$(TGTDLL)

$(INSTALLDIR)/$(TGTDBG): $(TGTDLL)
	$(CP) $(TGTDBG) $(INSTALLDIR)/$(TGTDBG)

$(TGTDLL): $(LIBFILES) $(COBJFILES) $(AOBJFILES) $(DEFFILE)
	$(CC) -shared -m32 -o $@ $(COBJFILES) $(AOBJFILES) $(DEFFILE) $(LDFLAGS)
	$(OBJCOPY) --only-keep-debug $(TGTDLL) $(TGTDBG)
	$(OBJCOPY) --strip-debug $(TGTDLL)

%.ao: %.asm
	$(ASM) $(AFLAGS) -o $@ $<

%.o: %.c ../manalink.h $(HFILES)
	$(COMPILE) -o $@ $<

lib%.a: %.def
	$(DLLTOOL) -k -d $< -l $@

testwhichgcc:
	@$(CC) -dumpmachine >/dev/null
	@$(ASM) --version >/dev/null

clean:
	$(RM) $(COBJFILES) $(AOBJFILES) $(TESTAOBJFILES) $(DEPFILES) $(LIBFILES) $(TGTDLL) $(TGTDBG) test.exe

-include $(DEPFILES)
