bin_PROGRAMS += profile

profile_LDFLAGS = $(AM_LDFLAGS)
profile_CFLAGS = $(AM_CFLAGS)
profile_CPPFLAGS = $(AM_CPPFLAGS) -I lib/
profile_LDADD = libprofile.a
profile_SOURCES = tools/builtin-test.c	\
				  tools/profile.c
