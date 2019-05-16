#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(perf-lib, 0.1)
AC_CONFIG_SRCDIR([lib/inst.h])
AC_CONFIG_HEADERS([config.h])

LT_INIT

AM_INIT_AUTOMAKE

AC_SUBST([PL_CFLAGS])
AC_SUBST([PL_LDFLAGS])

# Checks for programs.
AC_PROG_CC
AC_PROG_MKDIR_P

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([dlopen], [dl])
AC_SEARCH_LIBS([pow], [m])


# Checks for header files.
AC_CHECK_HEADERS([stdint.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Set default prefix to $(curdir)/build
PL_SET_DEFAULT_PREFIX

# Check debug
PL_CHECK_DEBUG


AC_CONFIG_FILES(Makefile)

AC_OUTPUT
