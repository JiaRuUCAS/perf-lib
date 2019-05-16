lib_LTLIBRARIES += libperf-profile.la

libperf_profile_la_CFLAGS = $(AM_CFLAGS) -shared -fpic
#libperf_profile_la_LDFLAGS = $(AM_LDFLAGS) -shared -fpic
libperf_profile_la_CPPFLAGS = $(AM_CPPFLAGS)
libperf_profile_la_include_HEADERS = lib/array.h	\
									 lib/inst.h		\
									 lib/util.h
libperf_profile_la_includedir = $(includedir)/profile
libperf_profile_la_SOURCES = lib/array.c
