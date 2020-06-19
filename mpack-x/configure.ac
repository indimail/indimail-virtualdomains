AC_PREREQ([2.52])
AC_INIT([mpack],1.6,[Manvendra Bhangui indimail-utils@indimail.org],mpack, [https://github.com/mbhangui/indimail-virtualdomains])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_CC_C99

case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	STRIP_OPT=-s
	;;
*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
esac
AC_SUBST(STRIP_OPT)

AC_CHECK_FUNCS([strchr])
AC_REPLACE_FUNCS([strcasecmp getopt])

AC_OUTPUT([Makefile])
