lib_LIBRARIES += libprofile.a

libprofile_a_CFLAGS = $(AM_CFLAGS)
#libperf_profile_la_LDFLAGS = $(AM_LDFLAGS) -shared -fpic
libprofile_a_CPPFLAGS = $(AM_CPPFLAGS)
libprofile_a_include_HEADERS = lib/array.h		\
							   lib/cmd.h		\
	  						   lib/inst.h		\
							   lib/util.h
libprofile_a_includedir = $(includedir)/profile
libprofile_a_SOURCES = lib/array.c		\
					   lib/cmd.c		\
					   lib/util.c
