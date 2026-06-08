# -*- makefile -*-

include ../Makefile-common

# Input files
BASEDIR = ..
SUBDIRS = src
LIBS = gdiplus gdi32
LFLAGS += -Wl,--enable-stdcall-fixup -Wl,--defsym,___ImageBase=__image_base__ \
	-Wl,--disable-dynamicbase -Wl,--disable-nxcompat

# Output files
OUTPUT = CardArtLib.dll

include ../Makefile-targets

install: ../../Cardartlib.dll

../../Cardartlib.dll: $(OUTPUT)
	/bin/cp -f CardArtLib.dll CardArtLib.dbg ../..
