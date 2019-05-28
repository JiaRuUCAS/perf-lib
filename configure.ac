#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(profile, 0.1)
AC_CONFIG_SRCDIR([lib/inst.h])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIRS([m4])

LT_INIT

AM_INIT_AUTOMAKE

AC_SUBST([PF_CFLAGS])
AC_SUBST([PF_LDFLAGS])

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
PF_SET_DEFAULT_PREFIX

PF_SET_PAGE_SIZE

# Check debug
PF_CHECK_DEBUG

# Check kernel build environment
PF_CHECK_LINUX

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(kmod/Makefile)

AC_OUTPUT
