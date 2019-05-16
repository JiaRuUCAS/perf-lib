AUTOMAKE_OPTIONS = foreign subdir-objects
#SUBDIRS = \
#		  ipsec

AM_CFLAGS = -Wstrict-prototypes
AM_CFLAGS += -Werror
AM_CFLAGS += -O2 -g -Wall
AM_CFLAGS += $(PL_CFLAGS)
#AM_CFLAGS += -D_FILE_OFFSET_BITS=64

#PL_CFLAGS += -Wall -Wstrict_prototypes

AM_LDFLAGS = $(PL_LDFLAGS)

if PL_DEBUG
AM_CPPFLAGS = -DPL_DEBUG
else
AM_CPPFLAGS =
AM_CFLAGS += -fomit-frame-pointer
endif


bin_PROGRAMS =
lib_LTLIBRARIES =

include lib/automake.mk
include test/automake.mk
