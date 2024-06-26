dnl vim: set noai tw=0:
dnl
dnl configure.ac for bogofilter
dnl (C) Copyright 2003 Clint Adams, Gyepi Sam, David Relson, Matthias Andree
dnl (C) Copyright 2008 Clint Adams, David Relson, Matthias Andree
dnl (C) Copyright 2004-2007, 2009 David Relson, Matthias Andree
dnl
dnl ********************************************************
dnl "Magic" environment variables for this script are:
dnl
dnl BF_ZAP_LIBDB -  LIBDB turns out empty, so that
dnl set to "zap"    the database library can be taken from
dnl                 libs (for static linking).
dnl
dnl BF_FORCE_TRIO - Unconditionally compile and link against
dnl set to "yes"    the "Trio" [v]snprintf library (which is
dnl                 part of the bogofilter source).
dnl ********************************************************
dnl
AC_INIT([bogofilter],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include([conf-email])),indimail, [https://github.com/mbhangui/indimail-virtualdomains])
dnl
AC_PREREQ([2.68])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_SRCDIR([src/bogofilter.c])
AC_CANONICAL_HOST
dnl AM_INIT_AUTOMAKE([1.11 foreign tar-ustar parallel-tests no-dist-gzip dist-xz no-installinfo -Wall subdir-objects])
AM_INIT_AUTOMAKE([foreign parallel-tests no-installinfo subdir-objects])
AC_CONFIG_HEADERS([src/config.h:config.in])

AC_PROG_AWK
AC_PROG_CC(gcc xlc cc)
AC_PROG_CC_C99
AM_PROG_AR
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS
AC_CHECK_TOOL(AR, ar, no)
if test "$AR" = no ; then
  AC_MSG_ERROR([Please make sure that "ar" is in your PATH.])
fi
AC_PROG_RANLIB
AC_PATH_PROGS(PERL, perl perl5,,$PATH:/usr/local/bin:/usr/bin)
AC_PATH_PROGS(NSGMLS,onsgmls nsgmls,nsgmls)
AC_PATH_PROGS(RPMBUILD,rpmbuild rpm,rpmbuild)
AC_PATH_PROGS(XMLLINT,xmllint,xmllint)
AC_PATH_PROGS(FLEX,flex,false)
AM_PROG_LEX
if test "x$LEX" = xflex && test "$FLEX" != false ; then
  LEX="$FLEX"
  AC_DEFINE(LEX,,path to LEX)
fi

dnl Check compiler capabilities
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_C_RESTRICT

dnl define our test programs are C
AC_LANG([C])

if test "$ac_cv_c_inline" != no ; then
    AC_DEFINE(HAVE_INLINE,1, [define to 1 if 'inline' is supported])
    AC_SUBST(HAVE_INLINE)
fi

dnl Check for "extern inline", using a modified version
dnl of the test for AC_C_INLINE from acspecific.mt.
dnl Run this before AC_C_INLINE to avoid skewing the results?
dnl Imported from the GSL 1.4 documentation (section "Autoconf Macros")
dnl and modified to define HAVE_EXTERN_INLINE.
AC_CACHE_CHECK([for extern inline], ac_cv_c_extern_inline,
[ac_cv_c_extern_inline=no
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[extern $ac_cv_c_inline double foo(double x);
extern $ac_cv_c_inline double foo(double x) { return x+1.0; };
double foo (double x) { return x + 1.0; };]], [[  foo(1.0)  ]])],[ac_cv_c_extern_inline="yes"],[])
])

if test "$ac_cv_c_extern_inline" != no ; then
    AC_DEFINE(HAVE_EXTERN_INLINE,1,Define to 1 if your compiler supports extern inline storage class.)
    AC_SUBST(HAVE_EXTERN_INLINE)
fi

dnl Check for extension
AC_EXEEXT

dnl check for b0rked Solaris (and other shells) and find one that works
AC_MSG_CHECKING(for a working shell...)
for i in /usr/xpg6/bin/sh /usr/xpg4/bin/sh /usr/ccs/bin/sh /bin/sh /usr/bin/sh /bin/ksh /bin/bash /usr/local/bin/bash ; do
    $i -c 'if ! false ; then echo $(echo ok) ; fi' >config.$$ 2>/dev/null
    j=`cat config.$$`
    rm -f config.$$
    if test "x$j" = "xok" ; then
	SHELL=$i
	AC_SUBST(SHELL)
	break
    fi
done
AC_MSG_RESULT($SHELL)
if test "x$SHELL" = "x" ; then
    AC_MSG_ERROR(no SUS compliant shell found - on Solaris, install SUNWxcu4)
fi

dnl crutch for b0rked S/390 gcc:
if test "x$host_cpu" = "xs390" && test "x$GCC" = "xyes" ; then
    case ${CFLAGS} in
    *-O*)
	OCFLAGS="$CFLAGS"
	dnl override -Os and -O2 and higher to -O1.
	dnl this is imperfect, as you can fool it by saying -O04
	dnl however, you can as well say -O4 -O4 and it will
	dnl be -O1 and -O4 -- the last one takes precedence in gcc.
	CFLAGS="`echo \"$CFLAGS\" | sed 's/-O[[s1-9][0-9]]*/-O1/'`"
	export CFLAGS
	AC_MSG_NOTICE([GCC for s390: Overriding CFLAGS "$OCFLAGS" to "$CFLAGS".])
    esac
fi

dnl crutch for b0rked dgux gcc 2.7 (-g gives unresolved symbols in GSL),
dnl gcc 2.8.1 is fine according to Message-ID: <199805061718.NAA01272@monty>
dnl see http://list-archive.xemacs.org/xemacs-beta/199805/msg00291.html
case "$host_os" in
dgux*)
    if test "$GCC" = yes ; then
        case "`$CC --version`" in
        2.7*)
	    case ${CFLAGS} in
	    *-g*)
		OCFLAGS="$CFLAGS"
		CFLAGS=""
		for i in $OCFLAGS ; do
		    case x$i in
			x-g*) ;;
			*) CFLAGS="$CFLAGS$i " ;;
		    esac
		done
		AC_MSG_NOTICE([dgux: Removing -g options from CFLAGS, new: "$CFLAGS".])
		;;
	    esac
	    ;;
	esac
    fi
    ;;
esac

dnl If we're using gcc, enable some warnings
dnl missing: -Wunreachable-code
if test "$GCC" = yes ; then
    AC_MSG_CHECKING([if gcc is really Intel C++])
    case "`$CC -V 2>&1`" in
	[[iI][nN][tT][eE][lL]]*)	ICC=yes ;;
	*)			ICC=no ;;
    esac
    AC_MSG_RESULT($ICC)
fi
AC_CACHE_CHECK(for additional compiler options, ac_cv_prog_gcc_flags, [
ac_cv_prog_gcc_flags=""
if test "$GCC" = yes && test "$ICC" = no
then
  echo "void dummy(void);" >configure-dummy.c
  echo "void dummy(void) {}" >>configure-dummy.c
  addopts=
  # note: GCC 2.X print just the version number, like "2.95.3"
  #       GCC 3.X print something like "gcc (GCC) 3.3.1 (SuSE Linux)"
  case `$CC --version` in
      2.*) ;;
      *) addopts="$addopts -Wpointer-arith -ggdb" ;;
  esac
  for i in $addopts \
dnl the -ieee option is for Alpha, Clint Adams' recommendation
dnl See http://thread.gmane.org/gmane.mail.bogofilter.general/6352
      -mieee \
      -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wshadow \
      -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
      -Waggregate-return -Wmissing-declarations \
      -Wmissing-format-attribute -Wnested-externs \
      -fno-common -Wchar-subscripts -Wcomment \
      -Wimplicit -Wsequence-point -Wreturn-type \
      -Wno-system-headers -Wformat
      # -Wfloat-equal -Wformat=2
      # -Wmissing-noreturn -Wconversion -pedantic -Wredundant-decls
  do
       if $CC $i $ac_cv_prog_gcc_flags -c configure-dummy.c >/dev/null 2>&1 
       then ac_cv_prog_gcc_flags="$ac_cv_prog_gcc_flags $i"
       else echo "ignoring $i"
       fi
  done
fi])
rm -f configure-dummy.c configure-dummy.o
CFLAGS="$CFLAGS $ac_cv_prog_gcc_flags"
AC_SUBST(CFLAGS)

dnl Check if we're running GCC 4.0.0 or higher, if so, switch
dnl to the new -iquote scheme rather than using -I- -I.
gcc4=no
if test "$GCC" = yes && test "$ICC" = no ; then
    case `$CC -dumpversion 2>/dev/null` in
	""|2.*|3.*) ;;
	4.*) gcc4=yes ;;
    esac
    case "$CC" in
	*clang) gcc4=yes ;;
    esac
fi
AM_CONDITIONAL(USE_GCC4, test $gcc4 = yes)

AC_MSG_CHECKING(whether flex needs patching)
dnl Check whether flex needs patching
flex=no
if test -n "$LEX"; then
    case `$LEX --version` in
	*2.5.4) flex=254 ;;
	*2.5.31) flex=253x ;;
	*2.5.33) flex=253x ;;
	*2.5.35) flex=O.K. ;;
	*2.5.36*) flex=253n ;;
	*2.5.37*) flex=253n ;;
	*2.5.38*) flex=253n ;;
	*2.5.39*) flex=253n ;;
	*2.6.0*)  flex=253n ;;
	*2.6.*) flex=O.K. ;;
	*)	flex=unknown ;;
    esac
fi
AC_MSG_RESULT($flex)
AM_CONDITIONAL(PATCH_FLEX_254, test $flex = 254)
AM_CONDITIONAL(PATCH_FLEX_253X, test $flex = 253x)
AM_CONDITIONAL(PATCH_FLEX_253N, test $flex = 253n)

dnl large file support (LFS)
AC_SYS_LARGEFILE
dnl AC_FUNC_FSEEKO

dnl Checks for libraries.
AC_CHECK_LIB([m],  [cos])

dnl fdatasync can be faster than fsync but may be in librt on some systems
dnl such as Solaris. The code is currently stubbed out in db_lock.c
dnl AC_SEARCH_LIBS([fdatasync],[rt],AC_DEFINE(HAVE_FDATASYNC,1,[Define to 1 if you have the 'fdatasync' function.]))

AC_CHECK_DECLS([getopt,optreset],,,[[
#include <unistd.h>
/* Solaris */
#include <stdlib.h>
]])

dnl check for SYNC stuff
AC_CHECK_DECLS([O_DSYNC, O_SYNC, O_FSYNC],,,[#include <fcntl.h>])

dnl Coerce GLIBC 2.3 to GLIBC 2.2 compatibility.
dnl May not be complete with future extensions, but appears to work
dnl today, 2003-10-02. Matthias Andree
AC_DEFINE(__NO_CTYPE,1,[Define __NO_CTYPE to 1 to avoid GLIBC_2.3-specific ctype.h functions.])

###    use option --enable-indimail to compile in the INDIMAIL support
AC_ARG_ENABLE(indimail,
      [  --enable-indimail            compile in INDIMAIL protocol support],
      [with_INDIMAIL=$enableval],
      [with_INDIMAIL=no])
test "$with_INDIMAIL" = "yes" && AC_DEFINE(INDIMAIL,1,Define if you want INDIMAIL support compiled in)
AM_CONDITIONAL(INDIMAIL, test "$with_INDIMAIL" = yes)

servicedir="/service"
AC_ARG_ENABLE(servicedir, [  --enable-servicedir=dir         directory where supervise services are installed.],
    servicedir="$enableval",
    [
    if test "$servicedir" = ""
    then
        AC_MSG_ERROR([Unable to find your servicedir directory, specify --enable-servicedir.])
    fi
    ])
AC_DEFINE_UNQUOTED(SERVICEDIR,"$servicedir",supervise service directory)
AC_SUBST(servicedir)

dnl Allow the user to specify a header name to use to indicate whether a given
dnl message is SPAM or not.
AC_ARG_ENABLE(spam-header-name, 
	AS_HELP_STRING([--enable-spam-header-name=name],
	[use specified header name instead of "X-Bogosity"]),
	AC_DEFINE_UNQUOTED(SPAM_HEADER_NAME,
	["$enableval"], 
	[Use specified header name instead of "X-Bogosity"]),
	AC_DEFINE_UNQUOTED(SPAM_HEADER_NAME,
	["X-Bogosity"])
)

dnl Allow the user to enable/disable use of Unicode

AC_ARG_ENABLE(unicode,
	AS_HELP_STRING([--disable-unicode],
	[disables Unicode/UTF-8 conversion of character sets]),
	if test "$enableval" = no; then
		[USE_UNICODE=NO]
	fi
	if test "$enableval" = yes; then
		[USE_UNICODE=YES]
	fi
	if test x$USE_UNICODE != x ; then
		AC_DEFINE_UNQUOTED(USE_UNICODE, "$USE_UNICODE", [Unicode enabled/disabled flag])
	    	AC_SUBST(USE_UNICODE)
	fi
)

AM_CONDITIONAL(DISABLE_UNICODE, test x$USE_UNICODE = xNO)
AM_CONDITIONAL(ENABLE_UNICODE, test x$USE_UNICODE != xNO)

if test x$USE_UNICODE = xNO ; then
	AC_DEFINE(DISABLE_UNICODE,1,
	[Disable converting character sets to Unicode])
	[ENCODING="raw"]
else
	AM_ICONV
	AC_DEFINE(ENABLE_UNICODE,1,
	[Enable converting character sets to Unicode])
	[ENCODING="utf-8"]
fi
[DEFAULT_CHARSET="iso-8859-1"]

dnl Allow the user to enable Russian character set support

if test x$USE_UNICODE != xYES ; then
AC_ARG_ENABLE(russian,
	AS_HELP_STRING([--enable-russian],
	[switch default character set to CP866 [[iso-8859-1]]]),
	if test "$enableval" = no; then
		[USE_CP866=NO]
	else
		[USE_CP866=YES]
	fi
)

dnl Enable building with Russian character set support.
if test x$USE_CP866 = xYES ; then
	AC_DEFINE(CP866,1,
	[Default charset is CP866 rather than iso-8859-1])
	[DEFAULT_CHARSET="CP866"]
fi
fi

dnl Allow the user to specify a default charset
AC_ARG_WITH(charset,
	    AS_HELP_STRING([--with-charset=name],
	    [use specified charset instead of iso-8859-1 (overrides --enable-russian) [[iso-8859-1]]]),
	    [ DEFAULT_CHARSET=$withval ]
)

AC_SUBST(ENCODING)
AC_SUBST(DEFAULT_CHARSET)
AC_DEFINE_UNQUOTED(DEFAULT_CHARSET,
	["$DEFAULT_CHARSET"],
	[Use specified charset instead of iso-8859-1])

dnl Allow the user to enable memory usage debugging methods

AC_ARG_ENABLE(memdebug,
	AS_HELP_STRING([--enable-memdebug],
	[enable memory usage debugging]),
	if test "$enableval" = no; then
		[USE_MEMDEBUG=NO]
	else
		[USE_MEMDEBUG=YES]
	fi
)

dnl Enable building with memory usage debugging files.
if test x$USE_MEMDEBUG = xYES ; then
	AC_DEFINE(ENABLE_MEMDEBUG,1,
	[Enable memory usage debugging])
fi
AM_CONDITIONAL(ENABLE_MEMDEBUG, test x$USE_MEMDEBUG = xYES)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([sys/types.h sys/stat.h stdlib.h syslog.h sys/param.h fcntl.h string.h strings.h memory.h unistd.h sys/time.h sys/select.h inttypes.h stdarg.h stdint.h])
AC_CHECK_HEADERS([limits.h float.h],,[AC_CHECK_HEADERS(values.h)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_TYPES([uint, ulong, uint32_t, u_int32_t, int32_t, int16_t, u_int16_t, uint16_t, u_int8_t, ssize_t])
AC_CHECK_TYPE(u_long, unsigned long)
dnl XXX FIXME - this is a crude hack and not necessarily working,
dnl but it might be good enough if we don't see asynchronous signals
AC_CHECK_TYPES([sig_atomic_t],,,[
#include <signal.h>
])
dnl FreeBSD needs sys/types.h and sys/time.h for sys/resource.h
AC_CHECK_TYPES([rlim_t],,,[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>
])
AC_CACHE_SAVE
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
dnl The next one isn't really needed, but useful for post-mortem analysis of
dnl automated package build logs such as bento.freebsd.org or buildd.debian.org:
AC_CHECK_SIZEOF(size_t)

AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_VPRINTF

AC_CHECK_FUNCS(strchr strrchr memcpy memmove snprintf vsnprintf getopt_long arc4random)
AC_REPLACE_FUNCS(strlcpy strlcat strerror strtoul)

AC_LIB_RPATH

dnl Allow the user to enable/disable use of database transactions

AC_ARG_ENABLE(transactions,
	AS_HELP_STRING([--enable-transactions],
	[force transactional mode (disable traditional) in Berkeley DB @<:@compile both modes and select at run-time@:>@]),
	if test "$enableval" = no; then
		[USE_TRANSACTIONS=NO]
	fi
	if test "$enableval" = yes; then
		[USE_TRANSACTIONS=YES]
	fi
	if test x$USE_TRANSACTIONS != x ; then
		AC_DEFINE_UNQUOTED(USE_TRANSACTIONS, "$USE_TRANSACTIONS", [transactions enabled/disabled flag])
		AC_SUBST(USE_TRANSACTIONS)
	fi
)
dnl dummy ARG_ENABLE to produce the help string
AC_ARG_ENABLE(dummy,
	AS_HELP_STRING([--disable-transactions],
	[disable transactional mode (force traditional) in Berkeley DB @<:@compile both modes and select at run-time@:>@]),,)

AM_CONDITIONAL(DISABLE_TRANSACTIONS, test x$USE_TRANSACTIONS = xNO)
AM_CONDITIONAL(ENABLE_TRANSACTIONS, test x$USE_TRANSACTIONS = xYES)

if test x$USE_TRANSACTIONS = xNO ; then
	AC_DEFINE(DISABLE_TRANSACTIONS,1,
	[Disable transactional mode in Berkeley DB])
fi

if test x$USE_TRANSACTIONS = xYES ; then
	AC_DEFINE(ENABLE_TRANSACTIONS,1,
	[Force transactional mode in Berkeley DB])
fi

AC_ARG_ENABLE(dbshared-test,
	      AS_HELP_STRING([--disable-dbshared-test], [defeat test for shared environment (unsafe)]))

AC_CACHE_SAVE

WITH_DB_ENGINE=db
AC_ARG_WITH(database,
	    AS_HELP_STRING([--with-database=ENGINE],
	    [choose database engine {db|qdbm|sqlite3|tokyocabinet|kyotocabinet|lmdb} [[db]]]),
	    [ WITH_DB_ENGINE=$withval ]
)

case "x$WITH_DB_ENGINE" in
    xsqlite|xsqlite3)
	AC_DEFINE(ENABLE_SQLITE_DATASTORE,1, [Enable sqlite3 datastore])
	DB_TYPE=sqlite
	DB_EXT=.db
	AC_CHECK_HEADERS([sqlite3.h],,AC_MSG_ERROR(Cannot find sqlite3.h header file))
	AC_LIB_LINKFLAGS([sqlite3])
	LIBDB="$LIBSQLITE3"
	WITH_DB_ENGINE="sqlite3"
	saveLIBS="$LIBS"
	LIBS="$LIBDB $LIBS"
	AC_CHECK_FUNC([sqlite3_prepare_v2],,AC_DEFINE(sqlite3_prepare_v2,sqlite3_prepare,[Define to sqlite3_prepare if new interface missing (for sqlite < 3.3.9)]))
	LIBS="$saveLIBS"
	;;
    xtokyocabinet)
	AC_DEFINE(ENABLE_TOKYOCABINET_DATASTORE,1, [Enable tokyocabinet datastore])
	DB_TYPE=tokyocabinet
	DB_EXT=.tc
	AC_LIB_LINKFLAGS([tokyocabinet])
	LIBDB="$LIBTOKYOCABINET"
	saveLIBS="$LIBS"
	LIBS="$LIBS $LIBDB"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <tcutil.h>
#include <tcbdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
	], [
	    TCBDB *d = tcbdbnew();
	])],,AC_MSG_ERROR(Cannot link to tokyocabinet library.))
	LIBS="$saveLIBS"
        ;;
    xkyotocabinet)
	AC_DEFINE(ENABLE_KYOTOCABINET_DATASTORE,1, [Enable kyotocabinet datastore])
	DB_TYPE=kyotocabinet
	DB_EXT=.kct
	AC_LIB_LINKFLAGS([kyotocabinet])
	LIBDB="$LIBKYOTOCABINET"
	saveLIBS="$LIBS"
	LIBS="$LIBS $LIBDB"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <kclangc.h>
	], [
	    KCDB *db = kcdbnew();
	])],,AC_MSG_ERROR(Cannot link to kyotocabinet library.))
	LIBS="$saveLIBS"
        ;;
    xlmdb)
	AC_DEFINE(ENABLE_LMDB_DATASTORE,1, [Enable LMDB datastore])
	DB_TYPE=lmdb
	DB_EXT=.lmdb
	AC_LIB_LINKFLAGS([lmdb])
	LIBDB="$LIBLMDB"
	saveLIBS="$LIBS"
	LIBS="$LIBS $LIBDB"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <lmdb.h>
	], [
		MDB_env *env;
		MDB_txn *txn;
		MDB_dbi dbi;
		mdb_env_create(&env);
		mdb_env_set_maxreaders(env, 1);
		mdb_env_set_mapsize(env, 4096*42);
		mdb_env_open(env, "/tmp", 0, 0660);
		mdb_txn_begin(env, 0, 0, &txn);
		mdb_dbi_open(txn, "", 0, &dbi);
	])],,AC_MSG_ERROR(Cannot link to lmdb library.))
	LIBS="$saveLIBS"
        ;;
    xqdbm)
	AC_DEFINE(ENABLE_QDBM_DATASTORE,1, [Enable qdbm datastore])
	DB_TYPE=qdbm
	DB_EXT=.qdbm
	PKG_CHECK_MODULES(QDBM, qdbm,
		  [CFLAGS="$CFLAGS $QDBM_CFLAGS"],
		  [AC_LIB_LINKFLAGS([qdbm])
		   AC_CHECK_HEADERS(qdbm/depot.h,
				    CFLAGS="$CFLAGS -I/usr/include/qdbm")])
	LIBDB="$LIBQDBM $QDBM_LIBS"
	saveLIBS="$LIBS"
	LIBS="$LIBS $LIBDB"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdlib.h>
#include <depot.h>
	], [
	    DEPOT *d = dpopen("", 0, 0);
	])],,AC_MSG_ERROR(Cannot link to qdbm library.))
	LIBS="$saveLIBS"
    ;;
    xdb*)
	dnl check if we need LIBDB at all (it is harmful on BSD since
	dnl they rename the library including soname)
	dnl and try to guess if we need -lpthread or -lthread
	DB_TYPE=db
	DB_EXT=.db
	AC_LIB_LINKFLAGS([db])
	AC_MSG_CHECKING(db and dependent libraries)
	saveLIBS="$LIBS"
	havelib=0
	for dblibs in "" "$LIBDB" ; do
	    for addlibs in "" -lpthread -lthread ; do
		LIBDB="$dblibs $addlibs"
		export LIBDB
	        LIBS="$saveLIBS $LIBDB"
		export LIBS
		AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <db.h>], [int r, s, t; db_version(&r, &s, &t);])],havelib=1 ; break 2)
	    done
	done
	LIBS="$saveLIBS"
	export LIBS
	if test $havelib = 0 ; then
	    AC_MSG_RESULT(error)
	    AC_MSG_ERROR([[
=====================================================================
Cannot link against libdb? Try these steps to solve the problem:
1. Be sure that the Berkeley DB developer package is installed.
2. Type:  rm -f config.cache    then re-run configure.
3. If the problem persists, report it to the bogofilter mailing list.
   Remember to include your config.log.
=====================================================================
	    ]])
	fi
	AC_MSG_RESULT($LIBDB)
	LIBS="$saveLIBS"
	export LIBS
	AH_TEMPLATE([HAVE_DB_H], [Have suitable db.h header])
	AC_SUBST(LIBDB)

	dnl check if run-time link works
	saveLIBS="$LIBS"
	LIBS="$LIBS $LIBDB"
	AC_MSG_CHECKING(if a program can be linked against Berkeley DB and run)
	AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <db.h>
		   ]], [[
return 0;
		   ]])],,
		   [AC_MSG_RESULT(no)
	AC_MSG_ERROR([Cannot run a program linked against libdb.
Did you adjust your library search path to include your libdb?])],true)
	AC_MSG_RESULT(yes)

	dnl check if header and library of DB library match
	dnl ignore if cross compiling
	AC_MSG_CHECKING(if Berkeley DB header and library versions match)
	rm -r -f bfadtestdir
	mkdir bfadtestdir
	AC_RUN_IFELSE([
	AC_LANG_PROGRAM([[
		   #include <stdlib.h>
		   #include <stdio.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
		   #include <db.h>
		   ]], [[
			int maj, min;
			(void)db_version(&maj, &min, 0);
			(void)fprintf(stderr, "headers: %d.%d, library: %d.%d\n",
				DB_VERSION_MAJOR, DB_VERSION_MINOR, maj, min);
			if (maj != DB_VERSION_MAJOR) exit(1);
			if (min != DB_VERSION_MINOR) exit(1);
			exit(0);
		   ]])],,
		   [AC_MSG_RESULT(no)
		   AC_MSG_ERROR(db.h header file and db library version do not match.)],true)
	AC_MSG_RESULT(yes)
	AC_CHECK_DECLS(db_create,,
	       AC_MSG_ERROR([[Can not locate a suitable Berkeley DB db.h header file.
Use --with-libdb-prefix=PATH to specify the path to a v3+ install directory.
You can download Berkeley DB 4.3.x from http://www.sleepycat.com/]]),
	[[#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <db.h>]]
	)
	AC_MSG_CHECKING(if Berkeley DB supports shared environments)
	if test "x$enable_dbshared_test" != xno && test "x$USE_TRANSACTIONS" != xNO ; then
	AC_RUN_IFELSE([
	AC_LANG_PROGRAM([[
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#define TESTDIR "bfadtestdir"
#define barf(where, what) do { fprintf(stderr, "%s failed: %s\n", where, db_strerror(what)); exit(1); } while(0)
		   ]], [[
    DB_ENV *e;
    int r;
    if ((r = db_env_create(&e, 0))) barf("db_env_create", r);
    if ((r = e->open(e, TESTDIR, DB_INIT_LOG|DB_INIT_TXN|DB_INIT_MPOOL|DB_CREATE, 0664)))
	barf("DB_ENV->open", r);
    if ((r = e->close(e, 0)))
        barf("DB_ENV->close", r);
    if ((r = db_env_create(&e, 0))) barf("db_env_create", r);
    if ((r = e->remove(e, TESTDIR, 0))) barf("DB_ENV->remove", r);
    return 0;
		   ]])],,
		   [AC_MSG_RESULT(no)
	rm -r -f bfadtestdir
	AC_MSG_ERROR([
Cannot open a shared environment in Berkeley DB. This is a known problem
on Fedora Core Linux. Build your own Berkeley DB installation and use
--with-libdb-prefix to point bogofilter to it. Alternatively, you can
use the unsafe non-transactional database by configuring with
--disable-transactions.
Packagers, if you are absolutely sure that the target system supports
shared environments, you can use --disable-dbshared-test.])],true)
	rm -r -f bfadtestdir
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(skipped)
    fi

	AC_DEFINE(ENABLE_DB_DATASTORE,1, [Enable Berkeley DB datastore])
	LIBS="$saveLIBS"
    ;;
    *)
	AC_MSG_ERROR([Invalid --with-database argument. Supported engines are db, qdbm, sqlite3, tokyocabinet, kyotocabinet, lmdb.])
    ;;
esac

if test "a$STATIC_DB" = a ; then
    if test "a$WITH_DB_ENGINE" = a ; then
	STATIC_DB="-ldb"
    else
	STATIC_DB="-l$WITH_DB_ENGINE"
    fi
fi

if test "x$BF_ZAP_LIBDB" = "xzap" ; then
    LIBDB=
    export LIBDB
fi

AC_DEFINE_UNQUOTED(DB_EXT, "$DB_EXT", [database file extension])
AC_DEFINE_UNQUOTED(DB_TYPE, "$DB_TYPE", [database type])
AC_SUBST(DB_EXT)
AC_SUBST(DB_TYPE)
AC_SUBST(STATIC_DB)

AM_CONDITIONAL(ENABLE_QDBM_DATASTORE, test "x$WITH_DB_ENGINE" = "xqdbm")
AM_CONDITIONAL(ENABLE_SQLITE_DATASTORE, test "x$WITH_DB_ENGINE" = "xsqlite3")
AM_CONDITIONAL(ENABLE_TOKYOCABINET_DATASTORE, test "x$WITH_DB_ENGINE" = "xtokyocabinet")
AM_CONDITIONAL(ENABLE_KYOTOCABINET_DATASTORE, test "x$WITH_DB_ENGINE" = "xkyotocabinet")
AM_CONDITIONAL(ENABLE_LMDB_DATASTORE, test "x$WITH_DB_ENGINE" = "xlmdb")

dnl Use TRIO to replace missing snprintf/vsnprintf.
needtrio=0
if test "$BF_FORCE_TRIO" = "yes" ; then needtrio=1 ; ac_cv_func_vsnprintf=no ; ac_cv_func_snprintf=no ; fi
if test "x$ac_cv_func_snprintf" != "xyes" ; then
    AC_DEFINE(snprintf, trio_snprintf, [Define to trio_snprintf if your system lacks snprintf])
    needtrio=1
fi
if test "x$ac_cv_func_vsnprintf" != "xyes" ; then
    AC_DEFINE(vsnprintf, trio_vsnprintf, [Define to trio_vsnprintf if your system lacks vsnprintf])
    needtrio=1
fi

if test $needtrio -gt 0
then
    dnl BEGIN TRIO
    dnl Alpha floating-point compiler option.
    dnl
    AC_MSG_CHECKING(for IEEE compilation options)
    AC_CACHE_VAL(ac_cv_ieee_option, [
        ac_cv_ieee_option="none"

        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
        #if !(defined(__alpha) && (defined(__DECC) || defined(__DECCXX) || (defined(__osf__) && defined(__LANGUAGE_C__))) && (defined(VMS) || defined(__VMS)))
        # error "Option needed"
        #endif
        ]])],[ac_cv_ieee_option="/IEEE_MODE=UNDERFLOW_TO_ZERO/FLOAT=IEEE"],[])

        if test $ac_cv_ieee_option = none ; then
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
            #if !(defined(__alpha) && (defined(__DECC) || defined(__DECCXX) || (defined(__osf__) && defined(__LANGUAGE_C__))) && !(defined(VMS) || defined(__VMS)) && !defined(_CFE))
            # error "Option needed"
            #endif
            ]])],[ac_cv_ieee_option="-ieee"],[])
        fi

        if test $ac_cv_ieee_option = none ; then
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
            #if !(defined(__alpha) && (defined(__GNUC__) && (defined(__osf__) || defined(__linux__))))
            # error "Option needed"
            #endif
        ]])],[ac_cv_ieee_option="-mieee"],[])
        fi
    ]) dnl end AC_CACHE_VAL
    AC_MSG_RESULT($ac_cv_ieee_option)
    if test $ac_cv_ieee_option != none; then
        CFLAGS="${CFLAGS} ${ac_cv_ieee_option}"
    fi
    dnl END TRIO

    AC_DEFINE(NEEDTRIO, 1, [Define to 1 if trio is to be included.])
fi

AM_CONDITIONAL(NEEDTRIO, test "$needtrio" = "1")

dnl check if XMLTO is somewhere
AC_CHECK_PROG(ac_cv_enable_xmlto, xmlto, yes, no)
AC_MSG_CHECKING([whether xmlto is available])
AC_MSG_RESULT($ac_cv_enable_xmlto)

AM_CONDITIONAL(ENABLE_XMLTO, test x$ac_cv_enable_xmlto = xyes)
dnl end of tests

dnl BEGIN EMBEDDED GSL
AC_CACHE_CHECK([for extended floating point registers],ac_cv_c_extended_fp,
	       [case "$host" in
		*sparc*-*-*) ac_cv_c_extended_fp=no ;;     
		*powerpc*-*-*) ac_cv_c_extended_fp=no ;;      
		*mips*-*-*) ac_cv_c_extended_fp=no ;;
		*hppa*-*-*) ac_cv_c_extended_fp=no ;;      
		*alpha*-*-*) ac_cv_c_extended_fp=no ;;      
		*68k*-*-*) ac_cv_c_extended_fp=yes ;;      
		*86*-*-*) ac_cv_c_extended_fp=yes ;;      
		*) ac_cv_c_extended_fp=unknown ;;
	    esac])

if test $ac_cv_c_extended_fp != "no" ; then
  AC_DEFINE(HAVE_EXTENDED_PRECISION_REGISTERS,1)
  AC_SUBST(HAVE_EXTENDED_PRECISION_REGISTERS)
fi
dnl END EMBEDDED GSL

AH_BOTTOM([
#ifdef HAVE_EXTENDED_PRECISION_REGISTERS
#define GSL_COERCE_DBL(x) (gsl_coerce_double(x))
#else
#define GSL_COERCE_DBL(x) (x)
#endif
])


AC_CACHE_SAVE

if test "$sysconfdir" = "\${prefix}/etc" \
    && { test "$prefix" = "NONE" || test "$prefix" = "/usr" \
      || test "$prefix" = "/usr/" ; } ; then
    if test "$( (uname -s) )" = "Linux" ; then
      sysconfdir=/etc
    fi
    (
    prefix=$ac_default_prefix
    sysconfdir=`eval echo "$sysconfdir"`
    AC_MSG_NOTICE([configuration directory is "$sysconfdir".])
    AC_MSG_NOTICE([use --sysconfdir to override.])
    )
fi

dnl work around Linux ldconfig design bug
dnl ldconfig will cache libraries in order found in library search
dnl order rather than by minor version, so it will happily link against
dnl /usr/lib/libgsl.so.0 (which is 0.4.0) when
dnl /usr/local/lib/libgsl.so.0 (which is 0.5.0) is available
dnl so try to find gsl-config in trusted system path first

AC_MSG_NOTICE([-------------------------------------------------------------])
AC_MSG_NOTICE([If you have multiple versions of GSL installed, this])
AC_MSG_NOTICE([configure script will prefer the one in /lib or /usr/lib])
AC_MSG_NOTICE([to work around a bug in Linux ldconfig.])
AC_MSG_NOTICE([Deinstall the older GSL version if this behaviour annoys you.])
AC_MSG_NOTICE([-------------------------------------------------------------])

PATH_SAVED="$PATH"
PATH=/bin:/usr/bin:$PATH
export PATH

need_my_gsl=1
AC_ARG_WITH(included-gsl, AS_HELP_STRING([--with-included-gsl],[force use of shipped GSL rather than system GSL]),
    if test "x$withval" = "xyes" ; then need_my_gsl=2 ; fi)

if test "$need_my_gsl" = 1 ; then
  AM_PATH_GSL(1.4,
		AC_DEFINE(HAVE_GSL_14,,[Define if you have GNU Scientific Library 1.4 or newer])
		need_my_gsl=0)
  AM_PATH_GSL(1.0,
		AC_DEFINE(HAVE_GSL_10,,[Define if you have GNU Scientific Library 1.0 or newer])
		need_my_gsl=0)
fi

if test "$need_my_gsl" != 0 ; then
  GSL_LIBS="libbf_gsl.a"
  AC_DEFINE(HAVE_GSL_14)
  AC_DEFINE(HAVE_GSL_10)
fi

PATH="$PATH_SAVED"

AM_CONDITIONAL(NEED_GSL,test $need_my_gsl != 0)
AM_CONDITIONAL(NEED_GETOPT,test $ac_cv_func_getopt_long != yes)

have_dosish_system=no
case "${host}" in
    *-*-mingw32*)
        # special stuff for Windoze NT
        have_dosish_system=yes
        ;;
    i?86-emx-os2 | i?86-*-os2*emx )
        # OS/2 with the EMX environment
        have_dosish_system=no
        ;;
    i?86-*-msdosdjgpp*)
        # DOS with the DJGPP environment
        have_dosish_system=yes
        ;;
    *)
       ;;
esac

if test -f conf-version
then
	spamfilter_version=$(cat conf-version)
else
	spamfilter_version="1.1"
fi
AC_SUBST(spamfilter_version)

bogofilter_transform=`echo "${program_transform_name}" | sed -e 's,\\\\\\\\,\\\\,g;s,\\\$\\\$,\$,g'`
transformed_bogofilter=`echo bogofilter | sed -e "$bogofilter_transform"`
transformed_bogoutil=`echo bogoutil | sed -e "$bogofilter_transform"`
AC_SUBST(transformed_bogofilter)
AC_SUBST(transformed_bogoutil)

# Note the \\\\ for backslashes.  Autoconf eats one layer, leaving \\

AC_DEFINE(CURDIR_S, ".", [Define name of current directory (C string)])
AC_DEFINE(EXTSEP_S, ".", [Define file extension separator (C string)])

if test "$have_dosish_system" = no; then
AC_DEFINE(DIRSEP_C, '/', [Define directory separator (C character)])
AC_DEFINE(DIRSEP_S, "/", [Define directory separator (C string)])
else
AC_DEFINE(DIRSEP_C, '\\\\')
AC_DEFINE(DIRSEP_S, "\\\\")
fi

AC_CONFIG_FILES([Makefile \
		trio/Makefile \
		doc/Makefile \
		src/Makefile \
		src/tests/Makefile \
		src/bf_compact src/bf_copy src/bf_tar \
		])
AC_OUTPUT

AC_MSG_NOTICE([Using these settings:])
AC_MSG_NOTICE([---------------------])
AC_MSG_NOTICE([CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([  CFLAGS=$CFLAGS])
AC_MSG_NOTICE([ LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([    LIBS=$LIBS])
AC_MSG_NOTICE([   LIBDB=$LIBDB])
AC_MSG_NOTICE([GSL_LIBS=$GSL_LIBS])
AC_MSG_NOTICE([LIBICONV=$LIBICONV])
