dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2018 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT([courier-imap],[m4_normalize(m4_include(conf-version))],[courier-imap@lists.sourceforge.net])
AC_CONFIG_SRCDIR(libs/imap/imapd.c)
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([foreign])
>confdefs.h # Kill PACKAGE_ macros

AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT([dlopen])
AC_CONFIG_MACRO_DIR([m4])

AC_PATH_PROGS(PERL, perl5 perl, perl)
AC_PATH_PROGS(SETENV, env)

defaultprefix="/usr/lib/courier-imap"
AC_SUBST(defaultprefix)

CXXFLAGS="-std=c++0x $CXXFLAGS"
case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -Dlint -DLINUX -Wall"
	CFLAGS="$CFLAGS -fPIC -fpie"
	LDFLAGS="$LDFLAGS -pie"
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include"
	CXXFLAGS="$CXXFLAGS -DDARWIN -I/opt/local/include"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;
*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	;;
esac

PKG_CHECK_MODULES(LIBIDN2, libidn2 >= 0.0.0, [libidn=yes], [libidn=no])
if test "$libidn" != "yes"
then
	PKG_CHECK_MODULES(LIBIDN1, libidn >= 0.0.0, [libidn=yes], [libidn=no])
	if test "$libidn" != "yes"
	then
		AC_MSG_ERROR([libidn not found])
	fi
fi

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

if test x$exec_prefix = xNONE
then
	exec_prefix="$prefix"
fi

AC_ARG_WITH(certsdir, [ --with-certsdir Directory where certs are created ],
certsdir="$withval", certsdir="$datadir")

AC_SUBST(certsdir)

dnl Check whether we should use gdbm, or db.

needs_withdb=0

AC_ARG_WITH(db, [  --with-db=gdbm          Use the GDBM library.
  --with-db=db            Use the libdb.a library.],
	db="$withval", db=""; needs_withdb=1)

case "$db" in
gdbm)
	;;
db)
	;;
"")
	;;
*)
	AC_MSG_ERROR(Invalid --with-db option.)
	;;
esac

saveLIBS="$LIBS"

if test "$db" != "db"
then
	AC_CHECK_LIB(gdbm, gdbm_open, [ LIBGDBM=-lgdbm ; LIBS="-lgdbm $LIBS" ])
	AC_CHECK_FUNC(gdbm_open, [ HAVE_GDBM=y ])
fi

LIBS="$saveLIBS"

if test "$db" != "gdbm"
then

	AC_CHECK_LIB(db, dbopen, [ LIBDB=-ldb ; LIBS="-ldb $LIBS" ],
		AC_CHECK_LIB(db, db_open, [ LIBDB=-ldb ; LIBS="-ldb $LIBS"],
		AC_CHECK_LIB(db, db_env_create,
				[ LIBDB=-ldb; LIBS="-ldb $LIBS"])
	))

	AC_CHECK_FUNC(dbopen, HAVE_BDB=1)
	AC_CHECK_FUNC(db_open, HAVE_BDB=1)
	AC_CHECK_FUNC(db_env_create, HAVE_BDB=1)
fi

LIBS="$saveLIBS"

if test "$HAVE_GDBM$HAVE_BDB" = ""
then
	AC_MSG_ERROR(Cannot find either the gdbm or the db library.)
fi

USE_GDBM=0
USE_DB=0
if test "$HAVE_GDBM" = "y"
then
	if test "$needs_withdb" = 1
	then
		ac_configure_args="$ac_configure_args --with-db=gdbm"
	fi
	dblibrary=libs/gdbmobj/libgdbmobj.la
	USE_GDBM=1
	DBSUBDIR=libs/gdbmobj
else
	if test "$needs_withdb" = 1
	then
		ac_configure_args="$ac_configure_args --with-db=db"
	fi
	dblibrary=libs/bdbobj/libbdbobj.la
	USE_DB=1
	DBSUBDIR=libs/bdbobj
fi
if test -f /usr/bin/elinks
then
	links=/usr/bin/elinks
else
	links=/usr/bin/links
fi
AC_SUBST(links)
AC_SUBST(USE_GDBM)
AC_SUBST(USE_DB)
AC_SUBST(dblibrary)
AC_SUBST(DBSUBDIR)
AM_CONDITIONAL(HAVE_DBLIBRARY, test "$dblibrary" != "")

AC_ARG_WITH(userdb, [], userdb="$withval", userdb=/etc/userdb
	ac_configure_args="$ac_configure_args --with-userdb=$userdb")
AC_SUBST(userdb)
AC_ARG_WITH(makedatprog, [], ,
	ac_configure_args="$ac_configure_args --with-makedatprog='${libexecdir}/makedatprog'")

AC_ARG_WITH(mailuser, [], , ac_configure_args="$ac_configure_args --with-mailuser=$authmailuser")
AC_ARG_WITH(mailgroup, [], , ac_configure_args="$ac_configure_args --with-mailgroup=$authmailgroup")
AC_SUBST(authmailuser)
AC_SUBST(authmailgroup)

AC_ARG_WITH(plentyofbeer,
[  --enable-sendmail=prog       set sendmail location
  --enable-trusted-users='...' specify users allowed to use the -d option
  --enable-maildrop-uid=root   UID of maildrop
  --enable-maildrop-gid=mail   GID of maildrop
  --disable-authlib            Disable support for Courier Auth Library
  --enable-use-flock=(1|0)     enable flock()/lockf() locking
  --enable-use-dotlock=(1|0)   enable dotlock locking
  --enable-lockext-def=.lock   set extension for dotlock files
  --enable-locksleep-def=5     set sleep interval dotlock files (seconds)
  --enable-locktimeout-def=60  set timeout for dotlock files (seconds)
  --enable-lockrefresh-def=15  set timeout for dotlock files (seconds)
  --enable-tempdir=.tmp        directory for temp files (in user's home dir)
  --disable-tempdir            disable temp files in home directory (use
                               the tmpfile function)
  --enable-smallmsg=8192       size of buffer for small messages
  --enable-global-timeout=300  global watchdog timer (seconds)
  --enable-crlf-term=0         enable <CR><LF> line termination
  --enable-restrict-trusted=1  restrict -d for trusted users only
  --enable-keep-fromline=(1|0) preserve the original From_ line in messages
  --enable-syslog=1            log fatal errors to syslog
  --enable-authlib-tempreject=(1|0) whether authlib errors cause temprejects]
  --enable-dovecotauth         Enable Dovecot authentication)

AC_CONFIG_SUBDIRS(libs/numlib)
AC_CONFIG_SUBDIRS(libs/unicode)
AC_CONFIG_SUBDIRS(libs/bdbobj)
AC_CONFIG_SUBDIRS(libs/gdbmobj)
AC_CONFIG_SUBDIRS(libs/soxwrap)
AC_CONFIG_SUBDIRS(libs/rfc822)
AC_CONFIG_SUBDIRS(libs/rfc1035)
AC_CONFIG_SUBDIRS(libs/rfc2045)
AC_CONFIG_SUBDIRS(libs/md5)
AC_CONFIG_SUBDIRS(libs/sha1)
AC_CONFIG_SUBDIRS(libs/random128)
AC_CONFIG_SUBDIRS(libs/liblock)
AC_CONFIG_SUBDIRS(libs/maildir)
AC_CONFIG_SUBDIRS(libs/makedat)
AC_CONFIG_SUBDIRS(libs/libhmac)
AC_CONFIG_SUBDIRS(libs/waitlib)
AC_CONFIG_SUBDIRS(libs/tcpd)
AC_CONFIG_SUBDIRS(libs/rootcerts)
AC_CONFIG_SUBDIRS(libs/authlib)
AC_CONFIG_SUBDIRS(libs/imap)
AC_CONFIG_SUBDIRS(libs/maildrop)

mkdir -p m4

# Courier-IMAP standalone does not use SOCKSv5

AC_ARG_WITH(socks, [], , ac_configure_args="$ac_configure_args --without-socks")

# Neither does it use the change password feature

AC_ARG_WITH(authchangepwdir, [], ,
	ac_configure_args="$ac_configure_args --with-authchangepwdir=$libexecdir/imapmodules")

INSTALL_DATA='${INSTALL} -m 0600'
INSTALL_SCRIPT='${INSTALL}'

if ulimit -v 2>/dev/null
then
	ULIMIT='ulimit -v'
else
	ULIMIT='ulimit -d'
fi
AC_SUBST(ULIMIT)

#
# autoconf bug
#

ac_given_INSTALL="$INSTALL"

AC_ARG_WITH(package, [], package="$withval",
	[
		package="$PACKAGE"
		ac_configure_args="$ac_configure_args --with-package=$PACKAGE --with-version=$VERSION"
	])

AM_CONDITIONAL(HAVE_HTML, test -f ${srcdir}/html/main.html)
AM_CONDITIONAL(HAVE_SGML, test -d ${srcdir}/libs/docbook)

AC_CONFIG_FILES(Makefile dbobj.h dbobj.config imapd.rc imapd-ssl.rc
		pop3d.rc pop3d-ssl.rc makeimapaccess
		courier-imap.spec
		INSTALL.html
		packaging/suse/courier-imap.init
		packaging/systemd/courier-imap.sysvinit
		packaging/systemd/courier-imap.service)
AC_OUTPUT
