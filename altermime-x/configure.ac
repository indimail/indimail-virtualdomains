dnl Process this file with autoconf to produce a configure script.
AC_INIT([altermime],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-utils@indimail.org],altermime, [https://github.com/mbhangui/indimail-virtualdomains])
AM_CONFIG_HEADER(config.h)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign])
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_ISC_POSIX
CFLAGS="$CFLAGS -Wall -O3"
defaultprefix="/usr"
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_TRY_RUN( [

#include <stdio.h>

int main()
{
	printf("hello word\n");
	return(0);
}
],
    AC_MSG_RESULT(yes),
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.),
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.))

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_PROG_INSTALL

AC_OUTPUT(Makefile)
