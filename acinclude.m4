# -*- autoconf -*-

dnl PF_SET_DEFAULT_PREFIX
AC_DEFUN([PF_SET_DEFAULT_PREFIX],[
	prefix=`pwd`/build
	if test -d "$prefix"; then
		AC_MSG_NOTICE([Set default prefix to $prefix])
	else
		mkdir -p "$prefix"
		AC_MSG_NOTICE([Set default prefix to $prefix])
	fi])

dnl Checks for --enable-debug and defines PF_DEBUG if it is specified.
AC_DEFUN([PF_CHECK_DEBUG],
	[AC_ARG_ENABLE(
		[debug],
		[AC_HELP_STRING([--enable-debug],
			[Enable debugging features])],
		[case "${enableval}" in
			(yes) debug=true ;;
			(no)  debug=false ;;
			(*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		esac],
		[debug=false])
	AM_CONDITIONAL([PF_DEBUG], [test x$debug = xtrue])
])

dnl Checks for --with-linux and set kmod-related variables
AC_DEFUN([PF_CHECK_LINUX],
	[AC_ARG_WITH(
		[linux],
		[AC_HELP_STRING([--with-linux],
			[Specify the Linux kernel build directory])],
		[kernel_spec=true])

	AC_MSG_CHECKING([whether the kernel build directory is specified])
	if test "$kernel_spec" != true || test "$with_linux" = no; then
		AC_MSG_RESULT([no])
		KBUILD=/lib/modules/`uname -r`/build
		AC_MSG_NOTICE([Set kernel build directory to the default value $KBUILD])
	else
		AC_MSG_RESULT([yes])
		KBUILD=$with_linux
		AC_MSG_NOTICE([Set kernel build directory to $KBUILD])
	fi

	# checking kernel directory
	# Debian breaks kernel headers into "source" header and "build" headers.
	# We want the source headers, but $KBUILD give us build headers.
	AC_MSG_CHECKING([whether the kernel directory is correct])
	KSRC=$KBUILD
	if test -e $KSRC/include/linux/kernel.h; then
		AC_MSG_RESULT([yes])
	else
		# Debian kernel build Makefiles tend to include a line of the form:
		# MAKEARGS := -C /usr/src/linux-headers-3.2.0-1-common 0=/usr/src/linux-headers-3.2.0-1-486
		# First try to extract the source directory from this line
		KSRC=`sed -n 's/.*-C \([[^ ]]*\).*/\1/p' "$KBUILD"/Makefile`
		if test ! -e $KSRC/include/linux/kernel.h; then
			# Didn't work. Fall back to name-based heuristics that used to work
			case `echo "$KBUILD" | sed 's,/*$,,'` in # (
            	*/build)
              		KSRC=`echo "$KBUILD" | sed 's,/build/*$,/source,'`
              		;; # (
            	*)
              		KSRC=`(cd $KBUILD && pwd -P) | sed 's,-[[^-]]*$,-common,'`
              		;;
          	esac
		fi
		if test -e $KSRC/include/linux/kernel.h; then
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([$KBUILD is not a kernel build directory])
		fi
	fi
	AC_SUBST([KBUILD])
	# AC_SUBST([KSRC])
])
