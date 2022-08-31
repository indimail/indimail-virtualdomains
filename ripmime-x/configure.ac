#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([ripmime],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-utils@indimail.org],ripmime,[https://github.com/mbhangui/indimail-virtualdomains])
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([uuencode.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
CFLAGS="$CFLAGS -Wall -O3 -fPIC"
echo "host=[$host]"
case "$host" in
	*-*-darwin*)
	LIBICONV="-liconv"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;
esac
AC_SUBST([LIBICONV])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
