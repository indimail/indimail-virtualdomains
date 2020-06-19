dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([nssd], m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-auth@indimail.org],nssd, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT([disable-static])
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-auth@indimail.org")

dnl Checks for programs.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET

dnl Checks for header files.
AC_ISC_POSIX
AC_HEADER_STDC
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

AC_LIB_PREPARE_MULTILIB
RPATH_LIB="$acl_libdirstem"

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	OS=sunos
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
    LIBVER=1
	OS=solaris
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -Wno-enum-compare -s -O4 -Wall -fPIC"
	#STRIP_OPT=-s
    LIBVER=2
	OS=linux
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	STRIP_OPT=-s
	OS=darwin
	;;
freebsd*)
	INSTALL="/usr/bin/install -c"
    CPPFLAGS="$CPPFLAGS -DFREEBSD"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
    LIBVER=1
	OS=freebsd
    ;;
*)
	INSTALL="/usr/bin/install -c"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
esac

defaultprefix="/usr"

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi
AC_CHECK_HEADERS(syslog.h stdint.h shadow.h strings.h)
AC_CHECK_TYPES([socklen_t], , , [
#include <unistd.h>
#include <sys/socket.h>
])
AC_MSG_CHECKING("struct ucred\n")
AC_EGREP_HEADER([struct ucred], [sys/socket.h], have_ucred=yes)
dnl AC_EGREP_CPP([struct ucred], [
dnl				    #define _GNU_SOURCE
dnl				    #include <sys/socket.h>
dnl				], have_ucred=yes)
if test "$have_ucred" = "yes"; then
AC_DEFINE_UNQUOTED([HAVE_STRUCT_UCRED], 1, [Define if you have the struct ucred])
AC_MSG_RESULT(yes)
fi

AC_CHECK_FUNCS(strerror_r getgrouplist)
#FIND_MYSQL()
ACX_PTHREAD()

AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
  AC_MSG_ERROR(Could not find qmail library.)
fi
AC_CHECK_LIB(socket, getsockname)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(m, floor)
AC_CHECK_LIB(dl, dlsym)
AC_CHECK_LIB(z, compress)

controldir="$sysconfdir/control"
AC_ARG_ENABLE(controldir, [  --enable-controldir=dir       directory for control files.],
	controdir="$enableval",
	[
	 	if test "$domaindir" != ""
		then
			AC_DEFINE_UNQUOTED(CONTROLDIR,"$controldir", Directory for control files)
		fi
	])
AC_DEFINE_UNQUOTED(CONTROLDIR, "$controldir", Directory for control files)

saveCPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} -Dlint -I/usr/include/mysql -I/usr/include/qmail"
AC_CHECK_HEADERS([mysql.h])
AC_CHECK_TYPE([bool],AC_DEFINE(HAVE_BOOL, 1, have bool typedef),,[#include <mysql.h>])
CPPFLAGS=$saveCPPFLAGS

AC_ARG_ENABLE(dlload-mysql,   [  --enable-dlload-mysql=yes|no Turn off (n, default) or on (y) use dlopen to load libmysqlclient],
dlload_mysql="$enableval", 
[
case $dlload_mysql in
	1*|y*|Y*)
	dlload_mysql="yes"
	;;
	*)
	dlload_mysql="no"
	;;
esac
])

AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])
if test "$mysql_prefix" = ""
then
mysql_incdir=""
for f in /usr/local/mysql/include /usr/include/mysql
do
	if test -d $f
	then
		mysql_incdir=$f
		break
	fi
done
AC_ARG_ENABLE(mysqlincdir, [  --enable-mysqlincdir=""       directory where auth include files are.],
mysql_incdir="$enableval",
[
	if test "$mysql_incdir" = ""
	then
		AC_MSG_ERROR([Unable to find your inc dir, specify --enable-mysqlincdir.])
	fi
])

if test "$dlload_mysql" = "no"
then
mysql_libdir=""
for f in /usr/local/mysql/lib64/mysql /usr/local/mysql/lib64 /usr/local/mysql/lib/mysql /usr/local/mysql/lib /usr/lib/mysql
do
	if test -d $f
	then
		mysql_libdir=$f
		break
	fi
done
AC_ARG_ENABLE(mysqllibdir, [  --enable-mysqllibdir=path-to-auth-libs directory where auth libs are.],
mysql_libdir="$enableval",
[
	if test "$mysql_libdir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql libraries dir, specify --enable-mysqllibdir.])
	fi
])
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
AC_ARG_ENABLE(mysqlbindir, [  --enable-mysqlbindir=""       directory where mysql binary are.],
mysqlbindir="$enableval",
[
	if test "$mysqlbindir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql bin dir, specify --enable-mysqlbindir.])
	fi
])
mysql_prefix=`dirname $mysqlbindir`
else
	for f in $mysql_prefix/include/mysql.h $mysql_prefix/include/mysql/mysql.h
	do
		if test -f $f
		then
			mysql_incdir=`dirname $f`
			break
		fi
	done

	if test "$dlload_mysql" = "no"
	then
		if test -d $mysql_prefix/lib64/mysql
		then
			mysql_libdir=$mysql_prefix/lib64/mysql
		elif test -d $mysql_prefix/lib64
		then
			mysql_libdir=$mysql_prefix/lib64
		elif test -d $mysql_prefix/lib/mysql
		then
			mysql_libdir=$mysql_prefix/lib/mysql
		elif test -d $mysql_prefix/lib
		then
			mysql_libdir=$mysql_prefix/lib
		else
			mysql_libdir=$mysql_prefix/lib/mysql
		fi
		mysqlbindir=$mysql_prefix/bin
	fi
fi

if test "$dlload_mysql" = "no"
then
	saveLIBS="$LIBS"
	AC_CHECK_LIB(mysqlclient -L$mysql_libdir, mysql_real_connect,[AC_SUBST([LIB_MYSQL], ["-L$mysql_libdir -lmysqlclient"])], 
	AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
	dnl AC_CHECK_LIB(mysqlclient -L${mysql_libdir}, mysql_real_connect,libs="$libs -lmysqlclient", AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
	AC_CHECK_FUNC(mysql_optionsv, have_mysqloptionsv=1)
	LIBS=$saveLIBS
else
	AC_DEFINE_UNQUOTED(DLOPEN_LIBMYSQLCLIENT, "1", "if libmysqlclient is loaded using dlopen")
	AC_EGREP_HEADER(st_mysql_res, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_LIBMARIADB, 1, "have st_mysql_res typedef"),)
fi

auth_inc="-I/usr/include/qmail -I$mysql_incdir -I$mysql_incdir/mysql"
case "$host" in
	*-*-darwin*)
	if test "$dlload_mysql" = "no"
	then
		auth_libs="-L. $LIB_MYSQL $LIB_QMAIL"
	else
		auth_libs="-L. $LIB_QMAIL"
	fi
	;;
	*)
	if test "$dlload_mysql" = "no"
	then
		auth_libs="-L. $LIB_MYSQL $LIB_QMAIL"
	else
		auth_libs="-L. $LIB_QMAIL"
	fi
	;;
esac
#---------------------------------------------------------------------

AC_ARG_ENABLE(debug,          [--enable-debug                           Enable debug (see DEBUGGING)], [AC_DEFINE([DEBUG], 1, [Enable debug])])
defaultdomain=""
AC_ARG_ENABLE(default-domain, [--enable-default-domain=name             Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

socket_path=""
AC_ARG_ENABLE(nssd-socket,    [--enable-nssd-socket=unix domain socket path for nssd and client library],
socket_path="$enableval", 
[
	if test "$socket_path" = ""
	then
		socket_path="/tmp/nssd.sock"
	fi
])
AC_DEFINE_UNQUOTED(_PATH_NSVSD_SOCKET,"$socket_path",full path of the NSVS Socket File)

if test " $sysconfdir" = " \${prefix}/etc"; then
	config_path="$prefix/etc/nssd.conf"
else
	config_path="$sysconfdir/nssd.conf"
fi
AC_ARG_ENABLE(nssd-config-path, [--enable-nssd-config-path=$sysconfdir/nssd.conf     configuration file for nssd daemon.],
config_path="$enableval",
[
	if test "$config_path" = ""
	then
		config_path="$prefix/etc/nssd.conf"
	fi
])
AC_DEFINE_UNQUOTED(_PATH_NSVSD_CONF, "$config_path", full path of the NSVS configuration File)

AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER)
AC_SUBST(OS)
AC_SUBST(defaultdomain)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(auth_inc)
AC_SUBST(auth_libs)
(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Host               = $host"
echo "CPPFLAGS           = $CPPFLAGS"
echo "CXXFLAGS           = $CXXFLAGS"
echo "CFLAGS             = $CFLAGS"
echo "LDFLAGS            = $LDFLAGS"
echo "            Current settings"
echo "---------------------------------------"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
if test "$dlload_mysql" = "no"
then
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
else
echo "     MySQL lib dir = loaded using dlopen"
fi
echo "          data dir = $datarootdir --datarootdir=$datarootdir"
echo "       socket Path = $socket_path --enable-nssd-socket-path=$socket_path"
echo "       config Path = $config_path --enable-nssd-config-path=$config_path"
) | tee nssd.settings
AC_OUTPUT(Makefile)
