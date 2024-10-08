#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([logalert],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include([conf-email])),logalert, [https://github.com/mbhangui/indimail-virtualdomains])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LIBTOOL

AC_PREFIX_DEFAULT(/usr)

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi
CFLAGS="$CFLAGS -O4 -Wall"

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdarg.h stdlib.h string.h sys/time.h unistd.h getopt.h \
				  pwd.h grp.h inttypes.h limits.h sys/syslimits.h sys/param.h \
				  sys/wait.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

case "$host" in

*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS -I/usr/include/qmail"
	CFLAGS="$CFLAGS -O4 -Wall"
    ;;

*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4 -I/usr/include/qmail"
	CFLAGS="$CFLAGS -O4 -Wall"
    ;;

*-*-linux*)
	CPPFLAGS="$CPPFLAGS -Dlint -DLINUX -Wall -I/usr/include/qmail"
	CFLAGS="$CFLAGS -O4 -Wall"
    ;;

*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include -I/usr/local/include/qmail"
	CFLAGS="$CFLAGS -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
    ;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include -I/opt/local/include/qmail"
	CFLAGS="$CFLAGS -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;

*)
    AC_DEFINE(LINUX,,[Define if LINUX])
    ;;

esac

test "$prefix" = "NONE" && prefix=/usr
test "$exec_prefix" = "NONE" && exec_prefix='${prefix}'
test "$program_prefix" = "NONE" && program_prefix=
test "$program_suffix" = "NONE" && program_suffix=


AC_DEFINE_UNQUOTED([BINDIR], ["$prefix/bin"],
		   [DO NOT CHANGE THIS - this will be used by logalert in PARENT mode. If you would like to change it, please do it via configure script.])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm getcwd strchr strrchr])

AC_DEFINE_UNQUOTED(PREFIX, "$prefix", installation directory)

# Checks for libraries.
AC_CHECK_LIB(ssl, SSL_CTX_new, [AC_SUBST([LIB_SSL], ["-lssl -lcrypto"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl -lcrypto)
AC_CHECK_HEADERS([rpc/rpc.h],[have_rpc=yes],
  [have_rpc=no && AC_CHECK_LIB(tirpc, clnt_create, [have_tirpc=yes && AC_SUBST([LIB_RPC], ["-ltirpc"]) AC_DEFINE([HAVE_TIRPC], [1],[tirpc Library])],[have_tirpc=no],-ltirpc)])
AC_CHECK_LIB(tirpc, clnt_create, [have_tirpc=yes && AC_SUBST([LIB_RPC], ["-ltirpc"]) AC_DEFINE([HAVE_TIRPC], [1],[tirpc Library])],[have_tirpc=no],-ltirpc)

if test " $have_rpc" = " yes"
then
    AC_CHECK_HEADERS([rpc/types.h])
elif test " $have_tirpc" = " yes"
then
    CPPFLAGS="$CPPFLAGS -I/usr/include/tirpc $CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -I/usr/include/tirpc $CXXFLAGS"
    AC_CHECK_HEADERS([tirpc/rpc/rpc.h tirpc/rpc/types.h])
else
  AC_MSG_ERROR(Could not find any usable rpc library.)
fi
AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
  AC_MSG_ERROR(Could not find qmail library.)
fi

have_pcre=no
LIBPCREDIR=libpcre

AC_ARG_WITH(libpcre,
    [  --with-libpcre=DIR    Use an existing (compiled) pcre lib from DIR/include and DIR/lib. ],
    [  case "$with_libpcre" in
       yes)
       ;;
       *)
    CXXFLAGS="$CXXFLAGS -I$with_libpcre/include $CXXFLAGS"
    LDFLAGS="$LDFLAGS -L$with_libpcre/lib $LDFLAGS"
    have_pcre=yes
        ;;
  esac]
)

if test $have_pcre != yes ; then
	AC_CHECK_HEADER(pcre.h,
	  AC_CHECK_LIB(pcre, pcre_version, [have_pcre=yes ]),
	    [AC_CHECK_HEADERS(pcre.h,
            [AC_CHECK_LIB(pcre, pcre_version, [have_pcre=yes])]
           )]
        )

fi

if test $have_pcre != yes ; then
	echo "I could not find pcre library/headers, therefore I'm using"
	echo "regex.h header. Note that pcre is necessary to some funcionalities"
	echo "in logalert. See documentation for further details."
	echo "If you wish, visit http://www.pcre.org/"
	AC_CHECK_HEADERS([regex.h])
	AC_CHECK_FUNCS([regcomp])
else

	AC_DEFINE(HAVE_PCRE_H)
	LIBPCRE_LIBS="-lpcre"
	AC_SUBST(LIBPCRE_LIBS)
	AC_SUBST(LIBPCREDIR)
	AC_SUBST(PCRE_DEPENDS)
	AC_SUBST(PCRE_CLEAN)
	AC_SUBST(PCRE_DIST_CLEAN)

fi

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([logalert-release:logalert-release.in],[], [])
AC_OUTPUT
