AC_PREREQ([2.52])
AC_INIT([pam-multi],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include([conf-email])),pam-multi,[https://github.com/mbhangui/indimail-virtualdomains])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign])
LT_INIT
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST

AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_TARNAME)

AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC
dnl Checks for programs.
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_ISC_POSIX
AC_HEADER_STDC
saveCPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} -Dlint -I/usr/include/mysql -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include/qmail -I/usr/local/include/qmail"
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h netdb.h string.h strings.h sys/socket.h sys/types.h sys/stat.h])
AC_CHECK_HEADERS([sys/param.h fcntl.h syslog.h unistd.h stdarg.h errno.h crypt.h shadow.h dlfcn.h mysql.h])
AC_CHECK_HEADERS([pwd.h endian.h sys/endian.h])
AC_CHECK_TYPE([bool],AC_DEFINE(HAVE_BOOL, 1, have bool typedef),,[#include <mysql.h>])
CPPFLAGS=$saveCPPFLAGS

dnl AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_VOLATILE
AC_TYPE_PID_T

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	SYSTEM=SunOS
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	MODFLAGS="-g -O4 -fPIC -shared -Wall"
	SYSTEM=Solaris
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC -fpie"
	MODFLAGS="-g -module -O4 -fPIC -shared -Wall"
	LDFLAGS="$LDFLAGS -pie"
	SYSTEM=Linux
	;;
*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include -I/usr/local/include/qmail"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	MODFLAGS="-g -module -O4 -fPIC -shared -Wall"
	SYSTEM=FreeBSD
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include -I/opt/local/include/qmail"
	CFLAGS="$CFLAGS -g -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	MODFLAGS="-g -O3 -bundle -Wall -undefined dynamic_lookup"
	SYSTEM=Darwin
	;;
*)
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	MODFLAGS="-g -O4 -fPIC -shared -Wall"
	;;
esac

AC_CHECK_HEADERS([pam/pam_appl.h security/pam_appl.h pam_appl.h pam/pam_modules.h security/pam_modules.h pam_modules.h])

defaultprefix="/usr"
LIBVER=0
AC_SUBST(SYSTEM)
AC_SUBST(LIBVER)
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args --prefix=$prefix"
fi

AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
  AC_MSG_ERROR(Could not find qmail library.)
fi
AC_CHECK_LIB(dl, dlopen, [AC_SUBST([LIB_DL], ["-ldl"]) AC_DEFINE([HAVE_DL], [1],[Dynamic Link Library])],,-ldl)
AC_CHECK_LIB(pam, pam_start, [AC_SUBST([LIB_PAM], ["-lpam"]) AC_DEFINE([HAVE_PAM], [1],[PAM Library])],,-lpam)
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)

defaultdomain=""
AC_ARG_ENABLE(default-domain, [  --enable-default-domain=name  Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

moduledir=""
for f in /usr/lib64/security /usr/lib/security /usr/lib/pam /lib/*/security
do
	if test -d $f
	then
		moduledir=$f
		break
	fi
done
AC_ARG_ENABLE(moduledir, [  --enable-moduledir=dir specify PAM module installation path],
  moduledir="$enableval",
  [
	  if test "$moduledir" = ""
	  then
		  AC_MSG_ERROR([Unable to find your pam module dir, specify --enable-moduledir.])
	  fi
  ])
AC_DEFINE_UNQUOTED(PAM_MOD_DIR,"$moduledir",full path to PAM Module Directory)

pam_confdir=$prefix/etc/pam.d
for f in /etc/pam.d $prefix/etc/pam.d /usr/etc/pam.d
do
  if test -d $f
  then
    pam_confdir=$f
    break
  fi
done
AC_ARG_ENABLE(pamconfdir, [  --enable-pamconfdir=dir specify PAM config path],
  pam_confdir="$enableval",
  [
	  if test "$pam_confdir" = ""
	  then
		  AC_MSG_ERROR([Unable to find your pam configuration dir, specify --enable-pamconfdir.])
	  fi
  ])

AC_DEFINE_UNQUOTED(USE_MYSQL,$USE_MYSQL,Use MySQL for primary authentication)

controldir="$sysconfdir/control"
AC_ARG_ENABLE(controldir, [  --enable-controldir=dir       directory for control files.],
	controldir="$enableval",
	[
	 	if test "$controldir" != ""
		then
			AC_DEFINE_UNQUOTED(CONTROLDIR,"$controldir", Directory for control files)
		fi
	])
AC_DEFINE_UNQUOTED(CONTROLDIR, "$controldir", Directory for control files)

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
  fi
fi
mysqlbindir=$mysql_prefix/bin

if test "$dlload_mysql" = "no"
then
	saveLIBS="$LIBS"
	AC_CHECK_LIB(mysqlclient -L$mysql_libdir, mysql_real_connect,[AC_SUBST([LIB_MYSQL], ["-L$mysql_libdir -lmysqlclient"])], 
	AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
	dnl AC_CHECK_LIB(mysqlclient -L${mysql_libdir}, mysql_real_connect,libs="$libs -lmysqlclient", AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
	AC_CHECK_FUNC(mysql_optionsv, have_mysqloptionsv=1)
	LIBS=$saveLIBS
else
  AC_EGREP_HEADER(st_mysql_res, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_LIBMARIADB, 1, "have st_mysql_res typedef"),)
	AC_DEFINE_UNQUOTED(DLOPEN_LIBMYSQLCLIENT, "1", "if libmysqlclient is loaded using dlopen")
fi

AC_EGREP_HEADER(MYSQL_OPT_LOCAL_INFILE, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_LOCAL_INFILE, 1, "local INFILE support"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_VERIFY_SERVER_CERT, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT, 1, "verify server cert"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_KEY, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_KEY, 1, "ssl key"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CERT, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CERT, 1, "ssl cert"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CA, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CA, 1, "ssl ca"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CAPATH, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CAPATH, 1, "ssl capath"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CIPHER, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CIPHER, 1, "ssl cipher"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CRL, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CRL, 1, "ssl cert revocation list"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_CRLPATH, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_CRLPATH, 1, "ssl cert revocation list path"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_ENFORCE, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_ENFORCE, 1, "enforce ssl/tls"))
AC_EGREP_HEADER(MYSQL_OPT_TLS_VERSION, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_TLS_VERSION, 1, "tls version"))
AC_EGREP_HEADER(MYSQL_OPT_SSL_MODE, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_MYSQL_OPT_SSL_MODE, 1, "ssl mmode"))

AC_CHECK_LIB(m,floor)
AC_CHECK_LIB(z,compress)

HOST=$host
AC_SUBST(MODFLAGS)
AC_SUBST(moduledir)
AC_SUBST(controldir)
AC_SUBST(defaultdomain)
AC_SUBST(pam_confdir)
AC_SUBST(mysql_prefix)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(datarootdir)
AC_SUBST(HOST)

(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Host               = $host"
echo "CPPFLAGS           = $CPPFLAGS"
echo "CXXFLAGS           = $CXXFLAGS"
echo "CFLAGS             = $CFLAGS"
echo "LDFLAGS            = $LDFLAGS"
echo "MODFLAGS           = $MODFLAGS"
echo "            Current settings"
echo "---------------------------------------"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
echo "      MySQL Prefix = $mysqlprefix --enable-mysqlprefix=$mysql_prefix"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
if test "$dlload_mysql" = "no"
then
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
else
echo "     MySQL lib dir = loaded using dlopen"
fi
echo "MySQL bin directory= $mysqlbindir --enable-mysqlbindir=$mysqlbindir"
echo "         libraries = $libs $LIBS"
echo "  module directory = $moduledir"
echo " control directory = $controldir"
echo "    pam config dir = $pam_confdir"
echo "              Libs = $LIB_QMAIL $LIB_MYSQL $LIB_PAM $LIB_CRYPT $LIB_DL"
) | tee pam-multi.settings
AC_OUTPUT(Makefile)
AC_OUTPUT([pam-multi.spec])

dnl vim600: sts=2 sw=2 ts=2 et

