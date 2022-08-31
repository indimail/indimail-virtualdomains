dnl Process this file with autoconf to produce a configure script.
AC_INIT([altermime],[m4_normalize(m4_include(conf-version))],[Manvendra Bhangui indimail-utils@indimail.org],[altermime],[https://github.com/mbhangui/indimail-virtualdomains])
AM_CONFIG_HEADER(config.h)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign])
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
CFLAGS="$CFLAGS -Wall -O3"
defaultprefix="/usr"
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
