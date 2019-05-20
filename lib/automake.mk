lib_LIBRARIES += libprofile.a

libprofile_a_CFLAGS = $(AM_CFLAGS)
#libperf_profile_la_LDFLAGS = $(AM_LDFLAGS) -shared -fpic
libprofile_a_CPPFLAGS = $(AM_CPPFLAGS)
libprofile_a_include_HEADERS = lib/array.h		\
							   lib/bitmap.h		\
							   lib/cmd.h		\
							   lib/evlist.h		\
							   lib/evsel.h		\
	  						   lib/inst.h		\
							   lib/pmu.h		\
							   lib/threadmap.h	\
							   lib/util.h		\
							   lib/xyarray.h
libprofile_a_includedir = $(includedir)/profile
libprofile_a_SOURCES = lib/array.c		\
					   lib/bitmap.c		\
					   lib/cmd.c		\
					   lib/evlist.c		\
					   lib/evsel.c		\
					   lib/pmu.c		\
					   lib/threadmap.c	\
					   lib/util.c		\
					   lib/xyarray.h
