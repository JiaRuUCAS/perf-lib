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
