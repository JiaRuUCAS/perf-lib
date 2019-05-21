AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = kmod

AM_CFLAGS = -Wstrict-prototypes
AM_CFLAGS += -Werror
AM_CFLAGS += -O2 -g -Wall
AM_CFLAGS += $(PF_CFLAGS)
#AM_CFLAGS += -D_FILE_OFFSET_BITS=64

#PF_CFLAGS += -Wall -Wstrict_prototypes

AM_LDFLAGS = $(PF_LDFLAGS)

if PF_DEBUG
AM_CPPFLAGS = -DPF_DEBUG
else
AM_CPPFLAGS =
AM_CFLAGS += -fomit-frame-pointer
endif


bin_PROGRAMS =
lib_LIBRARIES =
lib_LTLIBRARIES =

modules_install:
	cd kmod && $(MAKE) modules_install

include lib/automake.mk
include tools/automake.mk
include test/automake.mk
