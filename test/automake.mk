bin_PROGRAMS += test-api

test_api_LDFLAGS = $(AM_LDFLAGS)
test_api_CFLAGS = $(AM_CFLAGS)
test_api_CPPFLAGS = $(AM_CPPFLAGS) -I lib/
test_api_SOURCES = test/test-api.c
test_api_LDADD = libperf-profile.la
