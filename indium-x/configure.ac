#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([indium],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indium@indimail.org],indium, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indium@indimail.org")
AM_INIT_AUTOMAKE

AC_LIB_PREPARE_MULTILIB
AC_CONFIG_MACRO_DIR([m4])
dnl Does not work on RHEL 5, CentOS 5 64 bit systems
RPATH_LIB="$acl_libdirstem"
if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-linux*)
	if test x"$FAKED_MODE" = x
	then
		idcommand="/usr/bin/id -u"
	else
		idcommand="/usr/bin/id -u mail"
	fi
	CPPFLAGS="$CPPFLAGS -Dlint -DLINUX -Wall"
	CFLAGS="$CFLAGS -O4 -fPIC -Wall -fno-strict-aliasing"
	;;
*-*-darwin*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include -I/opt/local/include/qmail"
	CFLAGS="$CFLAGS -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;
*-*-freebsd*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include -I/usr/local/include/tcl8.6"
	CFLAGS="$CFLAGS -O3 -Wall -fPIC"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	;;
*)
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
esac

defaultprefix="/usr"
LIBVER=0:0:0
AC_SUBST(LIBVER)
AC_SUBST(RPATH_LIB)
AC_SUBST(defaultprefix)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])
CPPFLAGS="${CPPFLAGS} -Dlint -I/usr/include/mysql -I/usr/local/include/mysql -I/usr/local/mysql/include"
AC_CHECK_HEADER(mysql.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset putenv strcasecmp strchr strerror strncasecmp strrchr])

if test -f /usr/lib/tclConfig.sh
then
	. /usr/lib/tclConfig.sh
elif test -f /usr/lib64/tclConfig.sh
then
	. /usr/lib64/tclConfig.sh
elif test -f /usr/local/lib/tclConfig.sh
then
	. /usr/local/lib/tclConfig.sh
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ltcl':
AC_CHECK_LIB([tcl8.6], [Tcl_Init])
# FIXME: Replace `main' with a function in `-ltk':
AC_CHECK_LIB([tk8.6], [Tk_Init])
dnl Checks for libraries.
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)


shareddir=$prefix
AC_ARG_ENABLE(shareddir, [  --enable-shareddir=dir       directory for shared files.],
	shareddir="$enableval",
	[
	 	if test "$shareddir" != ""
		then
			AC_DEFINE_UNQUOTED(SHAREDDIR,"$shareddir", Directory for shared files.)
		fi
	])
AC_DEFINE_UNQUOTED(SHAREDDIR, "$shareddir", Directory for shared files)
AC_SUBST(shareddir)

DATE=`date`
AC_SUBST(DATE)

AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])

mysqlbindir=""
for f in /usr/local/mysql/bin/mysql /usr/bin/mysql
do
	if test -f $f
	then
		mysqlbindir="`dirname $f`"
		break
	fi
done
	if test -d $mysql_prefix/lib64/mysql
	then
		mysql_libdir=$mysql_prefix/lib64/mysql
	elif
	test -d $mysql_prefix/lib64
	then
		mysql_libdir=$mysql_prefix/lib64
	elif
	test -d $mysql_prefix/lib/mysql
	then
		mysql_libdir=$mysql_prefix/lib/mysql
	elif
	test -d $mysql_prefix/lib
	then
		mysql_libdir=$mysql_prefix/lib
	elif
	test -d $mysql_prefix/mysql/lib
	then
		mysql_libdir=$mysql_prefix/mysql/lib
	else
		mysql_libdir=$mysql_prefix/lib/mysql
	fi
	mysqlbindir=""
	for f in /usr/local/mysql/bin/mysql /usr/bin/mysql
	do
		if test -f $f
		then
			mysqlbindir="`dirname $f`"
			break
		fi
	done

saveLIBS="$LIBS"
AC_CHECK_LIB(mysqlclient -L$mysql_libdir, mysql_real_connect,[AC_SUBST([LIB_MYSQL], ["-L$mysql_libdir -lmysqlclient"])], 
AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
LIBS=$saveLIBS

AC_SUBST(mysql_prefix)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
