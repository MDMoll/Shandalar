# -*- makefile -*-

include Makefile-common

# Flags
LFLAGS := $(LFLAGS) -Wl,-Tmanalink.lds --entry _DllEntry

# Input files
SUBDIRS = cards functions
EXCLUDED_CFILES += \
	cards/artifact_creatures.c \
	cards/artifacts.c \
	cards/black_creatures.c \
	cards/black_enchantments.c \
	cards/black_oneshot.c \
	cards/blue_creatures.c \
	cards/blue_enchantments.c \
	cards/blue_oneshot.c \
	cards/green_creatures.c \
	cards/green_enchantments.c \
	cards/green_oneshot.c \
	cards/jatill.c \
	cards/lands.c \
	cards/recoded_cards.c \
	cards/red_creatures.c \
	cards/red_enchantments.c \
	cards/red_oneshot.c \
	cards/white_creatures.c \
	cards/white_enchantments.c \
	cards/white_oneshot.c \
	functions/ability_icons.c \
	functions/affect_me.c
EXCLUDED_AFILES += \
	functions/get_card_instance.asm \
	functions/in_play.asm
AFILES := $(AFILES) ManalinkEh.asm
LIBS = gdi32 msimg32
LOCAL_LIBS = capturestackbacktrace Drawcardlib

# Output files
OUTPUT = ManalinkEh.dll

include Makefile-targets

clean::
	-$(RM) ManalinkEh.ao
