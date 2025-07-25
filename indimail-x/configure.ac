#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([IndiMail],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include([conf-email])),indimail, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CONFIG_HEADERS([config.h])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules no-dist])

# Checks for programs.
LT_INIT
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_CANONICAL_HOST
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	;;
*-*-solaris*)
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	;;
*-*-linux*)
	if test x"$FAKED_MODE" = x
	then
		idcommand="/usr/bin/id -u"
	else
		idcommand="/usr/bin/id -u mail"
	fi
	CPPFLAGS="$CPPFLAGS -Dlint -DLINUX -I/usr/include/qmail -Wall"
	EXTRACPPFLAGS="-Wformat=2 -Wformat-security -Wformat-signedness -Wformat-overflow -Wsuggest-attribute=format -Wmissing-format-attribute"
	CFLAGS="$CFLAGS -O4 -fPIC -fpie -Wall"
	LDFLAGS="$LDFLAGS -pie"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	SYSTEM=Linux
	;;
*-*-darwin*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include -I/opt/local/include/qmail"
	EXTRACPPFLAGS="-Wformat=2 -Wformat-security -Wmissing-format-attribute"
	CFLAGS="$CFLAGS -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	MODFLAGS="-g -O3 -bundle -Wall -undefined dynamic_lookup"
	SYSTEM=Darwin
	;;
*-*-freebsd*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include -I/usr/local/include/qmail"
	EXTRACPPFLAGS="-Wformat=2 -Wformat-security -Wmissing-format-attribute"
	CFLAGS="$CFLAGS -O3 -Wall -fPIC"
	MODFLAGS="-g -O3 -fPIC -shared -Wall"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	SYSTEM=FreeBSD
	;;
*)
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	;;
esac

# Checks for libraries.

# Checks for header files.
AC_PROG_EGREP

AC_CHECK_HEADERS([ctype.h fcntl.h stdlib.h string.h sys/wait.h sys/types.h sys/socket.h unistd.h time.h sys/time.h])
saveCPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} -Dlint -I/usr/include/mysql -I/usr/local/include/mysql -I/usr/local/mysql/include"
AC_CHECK_HEADERS([mysql.h pwd.h grp.h stdarg.h varargs.h getopt.h errno.h signal.h netdb.h netinet/in.h arpa/inet.h])
AC_CHECK_TYPE([bool],AC_DEFINE(HAVE_BOOL, 1, have bool typedef),,[#include <mysql.h>])
CPPFLAGS=$saveCPPFLAGS
AC_CHECK_HEADERS([dirent.h limits.h sys/ioctl.h net/if.h linux/sockios.h sys/sockio.h])
AC_CHECK_HEADERS([openssl/evp.h sys/syscall.h sys/select.h sys/un.h])
AC_CHECK_HEADERS([crypt.h inttypes.h])
AC_CHECK_HEADERS([udb.h elf.h magic.h a.out.h malloc.h sys/utsname.h sys/ipc.h asm/ioctls.h])
AC_CHECK_HEADERS([gsasl.h])
AC_CHECK_HEADERS([sodium_random.h])

AC_EGREP_HEADER(in_addr_t, netinet/in.h, AC_DEFINE(HAVE_IN_ADDR_T, 1, "have in_addr_t data type"),)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_GETGROUPS

# Checks for library functions.
AC_FUNC_CHOWN
AC_CHECK_FUNCS(strsignal fdatasync sigaction sigprocmask gethostname fchown getmntent getmntinfo setenv unsetenv rresvport)
defaultprefix="/usr"
if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args --prefix=$prefix"
fi

AC_CONFIG_SUBDIRS([eps-1.2])

AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
  AC_MSG_ERROR(Could not find qmail library.)
fi
AC_CHECK_LIB(socket, socket, [AC_SUBST([LIB_SOCKET], ["-lsocket"]) AC_DEFINE([HAVE_SOCKET], [1],[socket Library])],,-lsocket)
AC_CHECK_LIB(ssl, SSL_CTX_new, [AC_SUBST([LIB_SSL], ["-lssl -lcrypto"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl -lcrypto)
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)
AC_CHECK_LIB(ncurses, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-lncurses"]) AC_DEFINE([HAVE_TERMCAP_IN_NCURSES], [1],[ncurses Library])],notermcap=t,)
if  test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(ncurses5, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-lncurses5"]) AC_DEFINE([HAVE_TERMCAP_IN_NCURSES], [1],[ncurses Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(tinfo, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-ltinfo"]) AC_DEFINE([HAVE_TERMCAP], [1],[termcap Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(termcap, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-ltermcap"]) AC_DEFINE([HAVE_TERMCAP], [1],[termcap Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
   	AC_MSG_ERROR(Could not find termcap library.)
fi
AC_CHECK_LIB(ncurses, initscr, [AC_SUBST([LIB_CURSES], ["-lncurses"]) AC_DEFINE([HAVE_NCURSES], [1],[ncurses Library])],nocurses=t,)
if test " $nocurses" = " t"
then
	nocurses=""
	AC_CHECK_LIB(ncurses5, initscr, [AC_SUBST([LIB_CURSES], ["-lncurses5"]) AC_DEFINE([HAVE_CURSES], [1],[curses Library])],nocurses=t,)
	if test " $nocurses" = " t"
	then
		nocurses=""
		AC_CHECK_LIB(curses, initscr, [AC_SUBST([LIB_CURSES], ["-lcurses"]) AC_DEFINE([HAVE_CURSES], [1],[curses Library])],nocurses=t,)
	fi
	if test " $nocurses" = " t"
	then
    	AC_MSG_ERROR(Could not find curses/ncurses library.)
	fi
fi
AC_CHECK_LIB(gsasl, gsasl_init, [AC_SUBST([LIB_GSASL], ["-lgsasl"]) AC_DEFINE([HAVE_GSASL], [1],[GNU SASL Library])],,-lgsasl)
AC_CHECK_LIB(qmail, sodium_random, [AC_DEFINE([HAVE_SODIUM_RANDOM], [1],[sodium_random function])],,)

AC_DEFINE_UNQUOTED(USE_MYSQL,$USE_MYSQL,Use MySQL for primary authentication)

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
	for f in $mysql_prefix/include/mysql.h $mysql_prefix/include/mysql/mysql.h $mysql_prefix/mysql/include/mysql.h
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
		elif test -d $mysql_prefix/mysql/lib
		then
			mysql_libdir=$mysql_prefix/mysql/lib
		else
			mysql_libdir=$mysql_prefix/lib/mysql
		fi
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
fi
AC_DEFINE_UNQUOTED(MYSQLBINDIR,"$mysqlbindir",full path to MySQL binaries)
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
AC_EGREP_HEADER(st_mysql_res, $mysql_incdir/mysql.h, AC_DEFINE(HAVE_LIBMARIADB, 1, "have st_mysql_res typedef"),)

AC_ARG_ENABLE(passwd,   [  --enable-passwd=y|n           Turn on (y, default) or off (n) /etc/passwd (or shadow) authentication],
	ENABLE_PASSWD=$enableval,
	[
		ENABLE_PASSWD=y
	])
case $ENABLE_PASSWD in
1*|y*|Y*)
	ENABLE_PASSWD=1
	AC_CHECK_HEADERS(shadow.h)
	HAS_SHADOW_H=0
	AC_CHECK_HEADER(shadow.h,
	[
		HAS_SHADOW_H=1;
	])
	if test $HAS_SHADOW_H = 1
	then
		AC_CHECK_LIB(shadow, setspent)
	fi
	AC_DEFINE_UNQUOTED(ENABLE_PASSWD,$ENABLE_PASSWD,enable system passwds)
	;;
*)
	ENABLE_PASSWD=0
	;;
esac

if test -f /etc/shadow
then
	HAS_SHADOW=1
	AC_DEFINE_UNQUOTED(HAS_SHADOW,$HAS_SHADOW,/etc/shadow present)
fi

#
# pgconfig
#

AC_MSG_CHECKING(for pgconfig)

pgconfig=""

for f in /usr/local/pgsql/bin/pg_config /usr/local/bin/pg_config /usr/bin/pg_config
do
   if test -x $f; then
	  pgconfig=$f
	  AC_MSG_RESULT($pgconfig)
	  break
   fi
done

if test "$pgconfig" != ""; then
	AC_DEFINE_UNQUOTED(HAVE_PGSQL,$HAVE_PGSQL,Use PGSQL for authentication)
	HAVE_PGSQL=1
	auth_pg_libs="-L`$pgconfig --libdir` -lpq "
	AC_MSG_RESULT($auth_pg_libs)
	AC_SUBST(auth_pg_libs)
	auth_pg_inc="-I`$pgconfig --includedir`"
	AC_MSG_RESULT($auth_pg_inc)
	AC_SUBST(auth_pg_inc)
fi

###	use option --enable-mysql-escape to include the mysql-escape-string support
AC_ARG_ENABLE(mysql_escape,   [  --enable-mysql-escape=yes|no Turn on (y, default) or off (n) mysql escape string],
	mysql_escape=$enableval,
	[
		mysql_escape=y
	])
case $mysql_escape in
1*|y*|Y*)
	mysql_escape="yes"
	;;
*)
	mysql_escape="no"
	;;
esac
test "$mysql_escape" = "yes" && AC_DEFINE_UNQUOTED(mysql_query(x,y), _mysql_Query(x,y), mysql_real_escape_string)
case $have_mysqloptionsv in
	1*|y*|Y*)
	AC_DEFINE_UNQUOTED(HAVE_MYSQL_OPTIONSV,$have_mysqloptionsv, "Enable mysql_optionsv()")
	;;
esac

qmaildir=""
for f in /var/indimail /var/qmail
do
	if test -d $f
	then
		qmaildir=$f
	fi
done
AC_ARG_ENABLE(qmaildir, [  --enable-qmaildir=dir         directory where qmail is installed.],
	qmaildir="$enableval",
	[
		if test "$qmaildir" = ""
		then
			AC_MSG_ERROR([Unable to find your qmaildir directory, specify --enable-qmaildir.])
		fi
	])
AC_DEFINE_UNQUOTED(QMAILDIR,"$qmaildir",qmail's home directory)
AC_SUBST(qmaildir)

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

for f in /var/postfix
do
	if test -d $f
	then
		postfixdir=$f
	fi
done

AC_ARG_ENABLE(postfixdir, [  --enable-postfixdir=dir       directory where Postfix is installed.],
	postfixdir="$enableval",
	[
		if test "$postfixdir" != ""
		then
			AC_DEFINE_UNQUOTED(POSTFIXDIR,"$postfixdir",Directory where Postfix is installed)
		fi
	])

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

assigndir="$sysconfdir/users"
AC_ARG_ENABLE(assigndir, [  --enable-assigndir=dir       directory for qmail users/assign.],
	assigndir="$enableval",
	[
	 	if test "$assigndir" != ""
		then
			AC_DEFINE_UNQUOTED(ASSIGNDIR,"$assigndir", Directory for qmail users/assign)
		fi
	])
AC_DEFINE_UNQUOTED(ASSIGNDIR, "$assigndir",  Directory for qmail users/assign)
AC_DEFINE_UNQUOTED(LIBEXECDIR,"$libexecdir", Directory for architecture-dependent binaries)
AC_DEFINE_UNQUOTED(SYSCONFDIR,"$sysconfdir", Directory for configuration files)

domaindir=$prefix
AC_ARG_ENABLE(domaindir, [  --enable-domaindir=dir       directory for virtual domains.],
	domaindir="$enableval",
	[
	 	if test "$domaindir" != ""
		then
			AC_DEFINE_UNQUOTED(DOMAINDIR,"$domaindir", Directory for virtual domains)
		fi
	])
AC_DEFINE_UNQUOTED(DOMAINDIR, "$domaindir", Directory for virtual domains)

basepath="/home/mail"
AC_ARG_ENABLE(basepath,   [  --enable-basepath=dir         Base Path for User's Home Directories.],
	basepath="$enableval",
	[
		if test "$basepath" = ""
		then
				AC_MSG_ERROR([Unable to find your base path, specify --enable-basepath.])
		fi
	])
AC_DEFINE_UNQUOTED(BASE_PATH,"$basepath",Directory where User's Home Directories get created)
AC_SUBST(basepath)

logdir="/var/log/svc"
AC_ARG_ENABLE(logdir,   [  --enable-logdir=dir           Base Path for System logs.],
	logdir="$enableval",
	[
		if test "$logdir" = ""
		then
				AC_MSG_ERROR([Unable to find your logdir path, specify --enable-logdir.])
		fi
	])
AC_DEFINE_UNQUOTED(LOGDIR, "$logdir",Directory where System Logs get created)
AC_SUBST(logdir)

indiuser="indimail"
AC_ARG_ENABLE(indiuser, [  --enable-indiuser=indimail    user IndiMail was installed as.],
	indiuser="$enableval",
	[
		if test "$indiuser" = ""
		then
			AC_MSG_ERROR([Unable to find your indiuser user, specify --enable-indiuser.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIMAILUSER,"$indiuser",IndiMail User)
AC_SUBST(indiuser)
echo $indiuser > indiusername

indigroup="indimail"
AC_ARG_ENABLE(indigroup,[  --enable-indigroup=indimail   group IndiMail was installed as.],
	indigroup="$enableval",
	[
		if test "$indigroup" = ""
		then
			AC_MSG_ERROR([Unable to find your indigroup group, specify --enable-indigroup.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIGROUP,"$indigroup",IndiMail Group)
AC_SUBST(indigroup)
AC_SUBST(indimailuid)
AC_SUBST(indimailgid)

AC_MSG_CHECKING(for indimail user!!!)

AC_RUN_IFELSE([AC_LANG_SOURCE([[

#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
struct passwd *pw;
char tmpbuf[100];
FILE *f;

	unlink("indimail.uid");
	unlink("indimail.gid");

	if ( (f=fopen("indiusername","r+"))==NULL) {
		fprintf(stderr, "could not open indiusername file\n");
		fprintf(stdout, "could not open indiusername file\n");
		exit(-1);
	}
    fgets(tmpbuf,100,f);
    fclose(f);
    tmpbuf[strlen(tmpbuf)-1] = 0;
	if (( pw=getpwnam(tmpbuf)) != 0 ) {
		f=fopen("indimail.dir","a+");
		fprintf(f,"%s\n", pw->pw_dir);
		fclose(f);

		f=fopen("indimail.uid", "a+");
		fprintf(f,"%d\n", pw->pw_uid);
		fclose(f);

		f=fopen("indimail.gid", "a+");
		fprintf(f,"%d\n", pw->pw_gid);
		fclose(f);
	} else {
		f=fopen("indimail.dir", "a+");
		fprintf(f,"/var/indimail\n");
		fclose(f);
		f=fopen("indimail.uid", "a+");
		fprintf(f,"555\n");
		fclose(f);

		f=fopen("indimail.gid", "a+");
		fprintf(f,"555\n");
		fclose(f);
	}
	return(0);
}
]])],[AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)],[])

if test ! -f indimail.uid
then
	AC_MSG_ERROR(No indimail user.)
fi

if test ! -f indimail.gid
then
	AC_MSG_ERROR(No indimail group.)
fi

if test ! -f indimail.dir
then
	indimaildir=$qmaildir
else
	indimaildir=`cat indimail.dir`
fi
if test -n "$EXTRACPPFLAGS"
then
	AC_MSG_CHECKING(extra cppflags)
	echo "int main() { return 0; }" >/tmp/testccflags.c
	flags=""
	found=0
	for i in ${EXTRACPPFLAGS}
	do
		${CC} -Wformat=2 $i -c /tmp/testccflags.c 2>/dev/null
		if test $? -eq 0
		then
			found=1
			flags="$flags $i"
		fi
	done
	if test $found -eq 1
	then
		CPPFLAGS="$CPPFLAGS -Wformat=2 $flags"
	fi
	echo done
	/bin/rm -f testccflags.o
fi

DATE=`date`
AC_SUBST(DATE)
AC_SUBST(indimaildir)
AC_SUBST(controldir)
AC_SUBST(domaindir)
AC_SUBST(assigndir)
AC_DEFINE_UNQUOTED(INDIMAILDIR,"$indimaildir",IndiMail Home Directory)
AC_DEFINE_UNQUOTED(PREFIX,"$prefix",Prefix)
rm -f indimail.dir

indimailuid=`cat indimail.uid`
rm -f indimail.uid

indimailgid=`cat indimail.gid`
rm -f indimail.gid
rm -f indiusername

defaultdomain=""
AC_ARG_ENABLE(default-domain, [  --enable-default-domain=name  Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_SUBST(defaultdomain)
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

moduledir=""
for f in /usr/lib64/security /usr/lib/security /usr/lib/pam
do
	if test -d $f
	then
		moduledir=$f
		break
	fi
done
AC_ARG_ENABLE([moduledir], [  --enable-moduledir=DIR specify PAM module installation path],
moduledir="$enableval"
[
	if test "$moduledir" = ""
	then
		AC_MSG_ERROR([Unable to find your pam module dir, specify --enable-moduledir.])
	fi
])
AC_DEFINE_UNQUOTED(PAM_MOD_DIR,"$moduledir",full path to PAM Module Directory)
AC_SUBST(moduledir)

log_name="indimail"
AC_ARG_ENABLE(log-name, [  --enable-log-name=indimail    set syslog name.], log_name="$enableval", )
AC_DEFINE_UNQUOTED(LOG_NAME,"$log_name",name to use for syslog)

AC_ARG_ENABLE(roaming-users, [  --enable-roaming-users=n|y    Turn on (y, default) or off (n) open relay after POP/IMAP authentication],
	POPRELAY=$enableval,
	[
		POPRELAY=yes
	])
case $POPRELAY in
1*|y*|Y*)
	POPRELAY=1
	AC_DEFINE_UNQUOTED(POP_AUTH_OPEN_RELAY,$POPRELAY,Enable POP/IMAP before SMTP)
	for f in /usr/local/bin/tcprules /usr/bin/tcprules /var/qmail/bin/tcprules
	do
		if test -f $f
		then
			tcprules_prog=$f
		fi
	done

	relay_clear_minutes="60"
	AC_ARG_ENABLE(relay-clear-minutes, [  --enable-relay-clear-minutes=60 expire time for roaming users after POP/IMAP authentication.],
    	relay_clear_minutes="$enableval", )
	AC_DEFINE_UNQUOTED(RELAY_CLEAR_MINUTES,$relay_clear_minutes,Interval after which Relay Permission is Reset)

	AC_ARG_ENABLE(tcprules-prog, [  --enable-tcprules-prog=/var/qmail/bin/tcprules where is your tcprules program.],
	tcprules_prog="$enableval",
	[
		if test "$tcprules_prog" = ""
		then
			AC_MSG_ERROR([Unable to find your tcprules prog, specify --enable-tcprules-prog.])
		fi
	] )
	AC_DEFINE_UNQUOTED(TCPRULES_PROG,"$tcprules_prog",full path of tcprules program)

	tcpserver_file=""
	for f in "$sysconfdir"/tcp/tcp.smtp "$sysconfdir"/tcp.smtp /etc/tcp.smtp /etc/tcprules.d/qmail-smtpd
	do
	if test -f $f
	then
		tcpserver_file=$f
		break
	fi
	done

	AC_ARG_ENABLE(tcpserver_file, [  --enable-tcpserver-file=$sysconfdir/tcp/tcp.smtp  File where tcpserver -x relay information is stored.],
	tcpserver_file="$enableval",
	[
		if test "$tcpserver_file" = ""
		then
			echo "127.:allow,RELAYCLIENT=\"\"" > "$sysconfdir"/tcp/tcp.smtp
			tcpserver_file="$sysconfdir"/tcp/tcp.smtp
			AC_MSG_WARN([Unable to find your tcpserver relay file.])
			AC_MSG_WARN([Creating tcp.smtp in "$sysconfdir"/tcp/tcp.smtp.])
		fi
	])
	AC_DEFINE_UNQUOTED(TCP_FILE,"$tcpserver_file",full path of the TCP Rules File)

	open_smtp="$sysconfdir/tcp/open-smtp"
	AC_DEFINE_UNQUOTED(OPEN_SMTP,"$open_smtp","")
	;;
*)
	;;
esac

AC_ARG_ENABLE(mysql-logging, [  --enable-mysql-logging=y|n    Turn on (y) or off (n, default) logging detailed authentication attempts],
	ENABLE_MYSQL_LOGGING=$enableval,
	[
		ENABLE_MYSQL_LOGGING=n
	])
case $ENABLE_MYSQL_LOGGING in
1*|y*|Y*)
	ENABLE_MYSQL_LOGGING=1
	AC_DEFINE_UNQUOTED(ENABLE_MYSQL_LOGGING,$ENABLE_MYSQL_LOGGING,Enable logging in MySQL)
	;;
*)
	;;
esac

AC_ARG_ENABLE(auth-logging, [  --enable-auth-logging=y|n     Turn on (y, default) or off (n) logging authentication attempts],
	ENABLE_AUTH_LOGGING=$enableval,
	[
		ENABLE_AUTH_LOGGING=y
	])
case $ENABLE_AUTH_LOGGING in
1*|y*|Y*)
	ENABLE_AUTH_LOGGING=1
	AC_DEFINE_UNQUOTED(ENABLE_AUTH_LOGGING,$ENABLE_AUTH_LOGGING,Enable logging of Authentication time)
	;;
*)
	;;
esac

AC_ARG_ENABLE(hardquota,    [  --enable-hardquota=y|n        Turn on (y) or off (n, default) Hard quota],
	ENABLE_HARD_QUOTA=$enableval,
	[
		ENABLE_HARD_QUOTA=0
	])
case $ENABLE_HARD_QUOTA in
0*|n*|N*)
	;;
*)
	AC_DEFINE_UNQUOTED(HARD_QUOTA,"$ENABLE_HARD_QUOTA",Enable Hard Quotas)
	;;
esac

AC_ARG_ENABLE(maildir-quota,[  --enable-maildir-quota=y|n    Turn on (y, default) or off (n) support for maildir++ quota],
	USE_MAILDIRQUOTA=$enableval,
	[
		USE_MAILDIRQUOTA=y
	])
case $USE_MAILDIRQUOTA in
1*|y*|Y*)
	USE_MAILDIRQUOTA=1
	AC_DEFINE_UNQUOTED(USE_MAILDIRQUOTA,$USE_MAILDIRQUOTA,Enable Maildir Quota)
	;;
*)
esac

AC_ARG_ENABLE(large-site,   [  --enable-large-site=n|y       Turn on (y) or off (n, default) tune for large numbers of users per domain],
	LARGE_SITE=$enableval,
	[
		LARGE_SITE=no
	])
case $LARGE_SITE in
1*|y*|Y*)
	SITE_SIZE=1
	AC_DEFINE_UNQUOTED(SITE_SIZE,$SITE_SIZE,Enable Large Site)
	;;
*)
	SITE_SIZE=0
	AC_DEFINE_UNQUOTED(SITE_SIZE,$SITE_SIZE,Enable Small Site)
	;;
esac

AC_ARG_ENABLE(user-cluster, [  --enable-user-cluster=y|n     Turn on (y, default) or off (n) user cluster architecture],
	CLUSTERED_SITE=$enableval,
	[
	CLUSTERED_SITE=y
	])
case $CLUSTERED_SITE in
1*|y*|Y*)
    if test $SITE_SIZE -eq 1
	then
		AC_MSG_ERROR([--enable-large-site=y is incompatible with --enable-user-cluster=y.])
	else
	CLUSTERED_SITE=1
	AC_DEFINE_UNQUOTED(CLUSTERED_SITE,$CLUSTERED_SITE,Enable Clustering)
	fi
	;;
*)
	;;
esac

AC_ARG_ENABLE(qmail-ext,    [  --enable-qmail-ext=y|n        Turn on (y, default) or off (n) qmail email address extensions],
	QMAIL_EXT=$enableval,
	[
		QMAIL_EXT=y
	])
case $QMAIL_EXT in
1*|y*|Y*)
	QMAIL_EXT=1
	AC_DEFINE_UNQUOTED(QMAIL_EXT,$QMAIL_EXT,Enable qmail extension addresses)
	;;
*)
	;;
esac

AC_ARG_ENABLE(ip-alias-domains, [  --enable-ip-alias-domains=y|n Turn on (y) or off (n, default) virutal domain lookup via reverse ip address lookup for virtual domains],
	IPALIAS=$enableval,
	[
	IPALIAS=no
	])
case $IPALIAS in
1*|y*|Y*)
	IPALIAS=1
	AC_DEFINE_UNQUOTED(IP_ALIAS_DOMAINS,$IPALIAS,Enable IP Alias Domains)
	;;
*)
	;;
esac

AC_ARG_ENABLE(valias,           [  --enable-valias=n|y           Turn on (y, default) or off (n) enable mysql alias processing],
	VALIAS=$enableval,
	[
		VALIAS=y
	])
case $VALIAS in
1*|y*|Y*)
	VALIAS=1
	AC_DEFINE_UNQUOTED(VALIAS,$VALIAS,Enable Valias Mechanism)
	;;
*)
	;;
esac

AC_ARG_ENABLE(vfilter,          [  --enable-vfilter=n|y          Turn on (y) or off (n, default) enable mysql vfilter processing],
	VFILTER=$enableval,
	[
		VFILTER=n
	])
case $VFILTER in
1*|y*|Y*)
	VFILTER=1
	AC_DEFINE_UNQUOTED(VFILTER,$VFILTER,Enable Vfilters)
	;;
*)
	;;
esac

AC_ARG_ENABLE(file-locking, [  --enable-file-locking=n|y|s|l|f|c  Turn on (y, default) or off (n),
	(s) use semaphores, (l) use link, (f) use lockf, (c) use fcntl default],
	FILE_LOCKING=$enableval,
	[
		FILE_LOCKING=y
		USE_LOCKF=0
		USE_FCNTLLOCK=1
		USE_LINKLOCK=0
		USE_SEMAPHORES=0
	])
case $FILE_LOCKING in
1*|y*|Y*|c*|C*)
	FILE_LOCKING=c
	USE_LOCKF=0
	USE_FCNTLLOCK=1
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_FCNTLLOCK,$USE_FCNTLLOCK,Use fcntl for file locking)
	;;
f*|F*)
	FILE_LOCKING=f
	USE_LOCKF=1
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_LOCKF,$USE_LOCKF,Use lockf for file locking)
	;;
l*|L*)
	FILE_LOCKING=l
	USE_LOCKF=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=1
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_LINKLOCK,$USE_LINKLOCK,Use link for file locking)
	;;
s*|S*)
	FILE_LOCKING=s
	USE_LOCKF=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=1
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_SEMAPHORES,$USE_SEMAPHORES,Use semaphores for file locking)
	;;
*)
	FILE_LOCKING=0
	USE_LOCKF=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	;;
esac

AC_ARG_ENABLE(file-sync,        [  --enable-file-sync=y|n        Turn on (y, default) or off (n) file sync in mail delivery],
	FILE_SYNC=$enableval,
	[
		FILE_SYNC=y
	])
case $FILE_SYNC in
1*|y*|Y*)
	FILE_SYNC=1
	AC_DEFINE_UNQUOTED(FILE_SYNC,$FILE_SYNC,Enable fsync when writing mail to disk)
	;;
*)
	;;
esac

AC_ARG_ENABLE(make-seekable,    [  --enable-make-seekable=n|y    Turn on (y, default) or off (n) Make input to vdelivermail seekable by lseek/fseek],
	MAKE_SEEKABLE=$enableval,
	[
		MAKE_SEEKABLE=y
	])
case $MAKE_SEEKABLE in
1*|y*|Y*)
	MAKE_SEEKABLE=1
	AC_DEFINE_UNQUOTED(MAKE_SEEKABLE,$MAKE_SEEKABLE,Enable making seekable pipes)
	;;
*)
	;;
esac

AC_ARG_ENABLE(random-balancing, [  --enable-random-balancing=y|n Turn on (y) or off (n, default) Random Balancing of inquery queries],
	RANDOM_BALANCING=$enableval,
	[
		RANDOM_BALANCING=n
	])
case $RANDOM_BALANCING in
1*|y*|Y*)
	RANDOM_BALANCING=1
	AC_DEFINE_UNQUOTED(RANDOM_BALANCING,$RANDOM_BALANCING,Enable Random Loadbalancing of inquery)
	;;
*)
	;;
esac

AC_ARG_ENABLE(query-cache,     [  --enable-query-cache=y|n     Turn on (y) or off (n, default) Query Caching for authentication],
	QUERY_CACHE=$enableval,
	[
		QUERY_CACHE=n
	])
case $QUERY_CACHE in
1*|y*|Y*)
	QUERY_CACHE=1
	AC_DEFINE_UNQUOTED(QUERY_CACHE,$QUERY_CACHE,Enable Query Caching for authentication)
	;;
*)
	;;
esac

AC_ARG_ENABLE(domain-limits,    [  --enable-domain-limits=y|n    Turn on (y, default) or off (n) handling of Domain Limits],
	ENABLE_DOMAIN_LIMITS=$enableval,
	[
		ENABLE_DOMAIN_LIMITS=y
	])
case $ENABLE_DOMAIN_LIMITS in
1*|y*|Y*)
	ENABLE_DOMAIN_LIMITS=1
	AC_DEFINE_UNQUOTED(ENABLE_DOMAIN_LIMITS,$ENABLE_DOMAIN_LIMITS,Enable domain level limits)
	;;
*)
	;;
esac

dnl **********************************************************************
dnl Checks for IPv6
dnl **********************************************************************
dnl
dnl We test for IPv6 by checking, in turn, for availability of
dnl getaddrinfo, presence of the INET6 address/protocol family, and
dnl the existence of struct sockaddr_in6.  If any of them is missing,
dnl IPv6 is disabled, and the code reverts to old-style gethostbyname.
dnl
dnl If --enable-ipv6 is explicitly specified on the configure command
dnl line, we check for IPv6 and abort if not found.  If --disable-ipv6
dnl is specified, we disable IPv6 and don't check for it.  The default
dnl is to autodetect IPv6 and use it where available.
dnl

AC_ARG_ENABLE(ipv6,
  AS_HELP_STRING([--disable-ipv6],[disable IPv6 support]),
  [case "${enable_ipv6}" in
    n|no)
      AC_MSG_RESULT([disabling IPv6 at user request])
      dnl Disable IPv6 checking
      ipv6=no
      ;;
    y|yes)
      dnl IPv6 explicitly enabled: force its use (abort if unavailable).
      ipv6=yes
      force_ipv6=yes
      ;;
    auto)
      dnl Auto-detect IPv6, i.e. check for IPv6, but don't force it.
      ipv6=yes
      ;;
    *)
      AC_MSG_ERROR([Invalid --enable-ipv6 argument '$enable_ipv6'])
      ;;
    esac
  ], [
    dnl If nothing is specified, assume auto-detection.
    ipv6=yes
  ]
)

if test "X$ipv6" = "Xyes"; then
  AC_CHECK_FUNCS(getaddrinfo, [], [
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support getaddrinfo(3)])
    ipv6=no
  ])
fi

if test "X$ipv6" = "Xyes"; then
  PROTO_INET6([], [
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support the PF_INET6 protocol family])
    ipv6=no
  ])
fi

if test "X$ipv6" = "Xyes"; then
  TYPE_STRUCT_SOCKADDR_IN6([],[
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support 'struct sockaddr_in6'])
    ipv6=no
  ])
  if test "X$ipv6" = "Xyes"; then
    INDIMAIL_STRUCT_SOCKADDR_STORAGE
    MEMBER_SIN6_SCOPE_ID
  fi
fi
if test "X$ipv6" = "Xyes"; then
  AC_DEFINE([ENABLE_IPV6], 1, [Define if IPv6 support is enabled.])
  AC_MSG_RESULT([Enabling support for IPv6.])
elif test "x$force_ipv6" = "xyes"; then
  AC_MSG_ERROR([IPv6 support requested but not found; aborting])
fi

if test -f /usr/bin/lsb_release
then
  DISTRIB_ID=$(lsb_release -si)
  DISTRIB_VERSION=$(lsb_release -sr)
else
  DISTRIB_ID=$(sh ./lsb_release -si)
  DISTRIB_VERSION=$(sh ./lsb_release -sr)
fi
LIBVER=3:0:0
MODVER=3:0:0
AC_DEFINE_UNQUOTED(PREFIX, "$prefix",Installation Prefix)
AC_DEFINE_UNQUOTED(LIBVER, "$LIBVER", Library Version)
AC_DEFINE_UNQUOTED(MODVER, "$MODVER", iauth module version)
AC_SUBST(DISTRIB_ID)
AC_SUBST(DISTRIB_VERSION)
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(CFLAGS)
AC_SUBST(defaultprefix)
AC_SUBST(libdir)
AC_SUBST(mysql_prefix)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(mysqlbindir)
AC_SUBST(idcommand)
AC_SUBST(LIBVER)
AC_SUBST(MODFLAGS)
AC_SUBST(HOST)
AC_SUBST(SYSTEM)
AC_CONFIG_FILES([libindimail.pc:libindimail.pc.in],[], [])
AC_CONFIG_FILES([indimail-release:indimail-release.in],[], [])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Date                 = $DATE"
echo "Host                 = $host"
echo "DISTRIB_ID           = $DISTRIB_ID"
echo "DISTRIB_VERSION      = $DISTRIB_VERSION"
echo "ID command           = $idcommand"
echo "CPPFLAGS             = $CPPFLAGS"
echo "CXXFLAGS             = $CXXFLAGS"
echo "CFLAGS               = $CFLAGS"
echo "LDFLAGS              = $LDFLAGS"
echo "MODFLAGS             = $MODFLAGS"
echo "            Current settings"
echo "-------------------------------------------------------------"
echo "            Prefix = $prefix"
echo "   qmail directory = $qmaildir --enable-qmaildir=$qmaildir"
echo " control directory = $controldir --enable-controldir=$controldir"
echo " domains directory = $domaindir --enable-domaindir=$domaindir"
echo "        sysconfdir = $sysconfdir"
echo "        servicedir = $servicedir"
if test "$postfixdir" != ""
then
	echo " Postfix directory = $postfixdir --enable-postfixdir=$postfixdir"
fi
echo "    Mail Base Path = $basepath  --enable-basepath=$basepath"
echo "Supervise Log Path = $logdir    --enable-logdir=$logdir"
echo "    indimail  user = $indiuser  --enable-indiuser=$indiuser"
echo "    indimail group = $indigroup --enable-indigroup=$indigroup"
echo "indimail directory = $indimaildir"
echo "    indimail   uid = $indimailuid"
echo "    indimail   gid = $indimailgid"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
echo "   syslog log name = $log_name --enable-log-name=$log_name"
if test "$mysql_prefix" != ""
then
echo "      MySQL Prefix = $mysql_prefix"
fi
echo "MySQL bin directory= $mysqlbindir --enable-mysqlbindir=$mysqlbindir"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
if test "$dlload_mysql" = "no"
then
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
else
echo "     MySQL lib dir = loaded using dlopen"
fi
echo "            libdir = $libdir"
case $mysql_escape in
	1*|y*|Y*)
	echo "      mysql escape = enabled"
	;;
	2|n*|N*)
	echo "      mysql escape = disabled"
	;;
esac
case $have_mysqloptionsv in
	1*|y*|Y*)
	echo "  mysql_optionsv() = enabled"
	;;
esac
case $POPRELAY in
1*|y*|Y*)
	echo "     roaming users = ON --enable-roaming-users=y default"
	echo "  relay clear mins = $relay_clear_minutes --enable-relay-clear-minutes=$relay_clear_minutes"
	echo "    tcprules  prog = $tcprules_prog --enable-tcprules-prog=$tcprules_prog"
	echo "    tcpserver file = $tcpserver_file --enable-tcpserver-file=$tcpserver_file"
	echo "    open_smtp file = $open_smtp"
	;;
*)
	echo "     roaming users = OFF --enable-roaming-users=n"
	;;
esac

case $ENABLE_LOGGING in
1*|y*|Y*)
	echo "            syslog = show successful and failure login attempts --enable-logging=y"
	;;
2*|e*|E*)
	echo "            syslog = show only failure attempts --enable-logging=e default"
	;;
3*|p*|P*)
	echo "            syslog = show failure attempts with clear text password --enable-logging=p"
	;;
4*|v*|V*)
	echo "            syslog = log everything, including passwords in errors --enable-logging=v"
	;;
*)
	echo "            syslog = OFF"
	;;
esac

case $ENABLE_MYSQL_LOGGING in
1*|y*|Y*)
	echo "     mysql logging = ON  --enable-mysql-logging=y"
	;;
*)
	echo "     mysql logging = OFF --enable-mysql-logging=n default"
	;;
esac

case $ENABLE_AUTH_LOGGING in
1*|y*|Y*)
	echo "      auth logging = ON  --enable-auth-logging=y default"
	;;
*)
	echo "      auth logging = OFF --enable-auth-logging=n"
	;;
esac

case $ENABLE_HARD_QUOTA in
0*|n*|N*)
	echo "        user quota = OFF --enable-hardquota=n default"
	;;
*)
	echo "        user quota = $ENABLE_HARD_QUOTA --enable-hardquota=$ENABLE_HARD_QUOTA"
	;;
esac

case $USE_MAILDIRQUOTA in
1*|y*|Y*)
	echo "   maildir++ quota = ON  --enable-maildir-quota=y default"
	;;
*)
	echo "   maildir++ quota = OFF --enable-maildir-quota=n"
	;;
esac

case $LARGE_SITE in
1*|y*|Y*)
	echo "        Large Site = ON  --enable-large-site=y (multiple tables)"
	;;
*)
	echo "        Large Site = OFF --enable-large-site=n (indimail table) default"
	;;
esac

case $CLUSTERED_SITE in
1*|y*|Y*)
	echo "  Clustered   Arch = ON  --enable-user-cluster=y default"
	;;
*)
	echo "  Clustered   Arch = OFF --enable-user-cluster=n"
	;;
esac

case $QMAIL_EXT in
1*|y*|Y*)
	echo "address extentions = ON  --enable-qmail-ext=y default"
	;;
*)
	echo "address extentions = OFF --enable-qmail-ext=n"
	;;
esac

case $IPALIAS in
1*|y*|Y*)
	echo "          ip alias = ON  --enable-ip-alias-domains=y"
	;;
*)
	echo "          ip alias = OFF --enable-ip-alias-domains=n (default)"
	;;
esac

case $VALIAS in
1*|y*|Y*)
echo " valias processing = ON  --enable-valias=y default"
      ;;
*)
echo " valias processing = OFF --enable-valias=n"
	  ;;
esac

case $VFILTER in
1*|y*|Y*)
echo "vfilter processing = ON  --enable-vfilter=y"
      ;;
*)
echo "vfilter processing = OFF --enable-vfilter=n default"
	  ;;
esac

case $FILE_LOCKING in
1*|y*|Y*|c*|C*)
	echo "     fcntl locking = ON  --enable-file-locking=y default (use fcntl)"
	;;
f*|F*)
	echo "     lockf locking = ON  --enable-file-locking=f (use lockf)"
	;;
l*|L*)
	echo "      link locking = ON  --enable-file-locking=l (use link)"
	;;
s*|S*)
	echo " semaphore locking = ON  --enable-file-locking=s (use semaphore)"
	;;
*)
	echo "      file locking = OFF --enable-file-locking=n"
	;;
esac

case $FILE_SYNC in
1*|y*|Y*)
	echo "         file sync = ON  --enable-file-sync=y default"
	;;
*)
	echo "         file sync = OFF --enable-file-sync=n disable vdelivermail fsync"
	;;
esac

case $MAKE_SEEKABLE in
1*|y*|Y*)
	echo "make input seekable= ON  --enable-make-seekable=y default"
	;;
*)
	echo "make input seekable= OFF --enable-make-seekable=n"
	;;
esac

case $RANDOM_BALANCING in
1*|y*|Y*)
	echo "random     balance = ON  --enable-random-balancing=y"
	;;
*)
	echo "time slice balance = ON  --enable-random-balancing=n default"
	;;
esac

case $QUERY_CACHE in
1*|y*|Y*)
	echo "       Query cache = ON  --enable-query-cache=y"
	;;
*)
	echo "       Query cache = OFF --enable-query-cache=n default"
	;;
esac

case $ENABLE_PASSWD in
1*|y*|Y*)
echo "  system passwords = ON  --enable-passwd=y default"
      ;;
*)
echo "  system passwords = OFF --enable-passwd=n"
	  ;;
esac

case $ENABLE_DOMAIN_LIMITS in
1*|y*|Y*)
echo "     domain limits = ON  --enable-domain-limits=y default"
      ;;
*)
echo "     domain limits = OFF --enable-domain-limits=n"
	  ;;
esac

case $ipv6 in
1*|y*|Y*)
echo "      IPV6 Support = ON  --enable-ipv6=y default"
;;
*)
echo "      IPV6 Support = OFF --enable-ipv6=n"
AC_CHECK_LIB(nsl, gethostbyname,echo -lnsl >> socket.lib,,)
;;
esac

echo "-------------------------------------------------------------"
) | tee indimail.settings
