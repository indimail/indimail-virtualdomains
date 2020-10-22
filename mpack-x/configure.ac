AC_PREREQ([2.52])
AC_INIT([mpack],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-utils@indimail.org],mpack, [https://github.com/mbhangui/indimail-virtualdomains])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL

case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD"
	CFLAGS="$CFLAGS -O3 -Wall"
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O3 -Wall"
	;;
*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
esac

AC_CHECK_FUNCS([strchr])
AC_REPLACE_FUNCS([strcasecmp getopt])

AC_OUTPUT([Makefile])
