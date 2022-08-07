dnl Process this file with autoconf to produce a configure script.

AC_INIT([iwebadmin],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include(conf-email)),iwebadmin, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CANONICAL_TARGET
LT_INIT
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
dnl Needed for per-target flags
AM_PROG_CC_C_O
AC_CONFIG_MACRO_DIR([m4])

AC_ISC_POSIX

CPPFLAGS="${CPPFLAGS} -Dlint"

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
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DLINUX -I/usr/include/qmail -I/usr/include/indimail -I/usr/include/mysql"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC -fno-strict-aliasing"
	;;
*-*-darwin*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include -I/opt/local/include/qmail -I/usr/local/include/indimail"
	CPPFLAGS="$CPPFLAGS -I/opt/local/include/mysql"
	CFLAGS="$CFLAGS -O3 -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;
*-*-freebsd*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include -I/usr/local/include/qmail -I/usr/local/include/indimail"
	CPPFLAGS="$CPPFLAGS -I/usr/local/include/mysql"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	CFLAGS="$CFLAGS -O3 -Wall"
	;;
*)
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DLINUX -I/usr/include/qmail -I/usr/include/indimail -I/usr/include/mysql"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
esac

AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h signal.h unistd.h])
AC_CHECK_HEADERS([pwd.h ctype.h time.h stdlib.h sys/wait.h])
dnl Checks for libraries.
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)
AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
	AC_MSG_ERROR(Could not find qmail library.)
fi
AC_CHECK_LIB(indimail, iopen, [AC_SUBST([LIB_INDIMAIL], ["-lindimail"]) AC_DEFINE([HAVE_INDIMAIL], [1],[indimail Library])],noindimail=t,)
if test " $noindimail" = " t"
then
	AC_MSG_ERROR(Could not find indimail library.)
fi
AC_CHECK_LIB(indimail, gsasl_mkpasswd, [AC_DEFINE([HAVE_GSASL_MKPASSWD], [1],[gsasl_mkpasswd function])],,)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([dup2 memset mkdir strcasecmp strchr strdup strncasecmp strrchr strstr setreuid setregid])

CGIBINTARGETS=iwebadmin
AC_SUBST(CGIBINTARGETS)

AC_SUBST(idcommand)

defaultprefix="/usr"
if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args --prefix=$prefix"
fi

htmldir=""
for f in /var/www/html /home/httpd/html /usr/local/apache/htdocs /usr/local/www/data
do
	if test -d $f
	then
		htmldir=$f
	fi
done
AC_ARG_ENABLE(htmldir, [  --enable-htmldir=DIR      HTTP server's base html dir.], htmldir="$enableval",
	[
	if test "$htmldir" = ""
	then
		AC_MSG_ERROR([Unable to find your html base directory, specify --enable-htmldir.])
	fi
	])
AC_SUBST(htmldir)

cgibindir=""

for f in var/www /home/httpd /usr/local/apache /usr/local/www /var/lib/httpd /usr/local/www $htmldir
do
	if test -d $f/cgi-bin
	then
		cgibindir=$f/cgi-bin
	fi
done

AC_ARG_ENABLE(cgibindir, [  --enable-cgibindir=DIR    HTTP server's cgi-bin directory.],
	cgibindir="$enableval",
	[
	if test "$cgibindir" = ""
	then
		AC_MSG_ERROR([Unable to find your cgi-bin directory, specify --enable-cgibindir.])
	fi
	])
AC_SUBST(cgibindir)

imageurl="/images/iwebadmin"
AC_ARG_ENABLE(imageurl, [  --enable-imageurl=URL     URL to image files [/images/iwebadmin].],
	imageurl="$enableval",
	[
	if test "$imageurl" = ""
	then
		AC_MSG_ERROR([Unable to find your html url, specify --enable-imageurl.])
	fi
	])
AC_SUBST(imageurl)
AC_DEFINE_UNQUOTED(IMAGEURL, "$imageurl","URL to image files")

imagedir=""
for f in /var/www/html /home/httpd/html /usr/local/apache/htdocs /usr/local/www/data $htmldir
do
	if test -d $f
	then
		imagedir=$f$imageurl
	fi
done
AC_ARG_ENABLE(imagedir, [  --enable-imagedir=DIR     Where to put the image files [DocumentRoot/images/iwebadmin].],
	imagedir="$enableval",
	[
	if test "$imagedir" = ""
	then
		AC_MSG_ERROR([Unable to find your html image directory, specify --enable-imagedir.])
	fi
	])
AC_SUBST(imagedir)

htmllibdir="$prefix/share/iwebadmin"

AC_ARG_ENABLE(htmllibdir, [  --enable-htmllibdir=DIR   iwebadmin HTML library directory.],
	htmllibdir="$enableval",
	[
	if test "$htmllibdir" = ""
	then
		AC_MSG_ERROR([Unable to find your htmllibdir directory, specify --enable-htmllibdir.])
	fi
	])
AC_SUBST(htmllibdir,$htmllibdir)
AC_DEFINE_UNQUOTED(HTMLLIBDIR, "$htmllibdir","iwebadmin HTML library directory")


ezmlmdir=""
for f in /usr/bin /usr/local/bin
do
	if test -f $f/ezmlm-make
	then
		ezmlmdir=$f
	fi
done

AC_ARG_ENABLE(ezmlmdir, [  --enable-ezmlmdir=PATH    Path to ezmlm binaries.],
	ezmlmdir="$enableval",
	[
	if test "$ezmlmdir" = ""
	then
		AC_MSG_ERROR([Unable to find your ezmlmdir user, specify --enable-ezmlmdir.])
	fi
	])
AC_DEFINE_UNQUOTED(EZMLMDIR, "$ezmlmdir", "PATH to ezmlm binaries")

ezmlm_mysql=yes
AC_ARG_ENABLE(ezmlm-mysql, [  --disable-ezmlm-mysql         Disable MySQL options for mailing lists.], ezmlm_mysql="$enableval",)
case $ezmlm_mysql in
	1*|y*|Y*)
	AC_DEFINE_UNQUOTED(ENABLE_MYSQL, 1, "MySQL options for mailing lists")
	;;
	*)
	;;
esac

cgipath="/cgi-bin/iwebadmin"

AC_ARG_ENABLE(cgipath, [  --enable-cgipath=PATH     URL path for cgi [/cgi-bin/iwebadmin].],
	cgipath="$enableval",
	[
	if test "$cgipath" = ""
	then
		AC_MSG_ERROR([Unable to find your cgipath directory, specify --enable-cgipath.])
	fi
	])
AC_DEFINE_UNQUOTED(CGIPATH, "$cgipath", "URL path for CGI directory")

indimailuser="indimail"
AC_ARG_ENABLE(indimailuser, [  --enable-indimailuser=USER        User indimail was installed as [indimail].],
	indimailuser="$enableval",
	[
	if test "$indimailuser" = ""
	then
		AC_MSG_ERROR([Unable to find your indimailuser user, specify --enable-indimailuser.])
	fi
	])
AC_DEFINE_UNQUOTED(INDIMAILUSER, "$indimailuser", "indimail user name")
AC_SUBST(indimailuser)

indimailgroup="indimail"
AC_ARG_ENABLE(indimailgroup, [  --enable-indimailgroup=GROUP      Group indimail was installed as [indimail].],
	indimailgroup="$enableval",
	[
	if test "$indimailgroup" = ""
	then
		AC_MSG_ERROR([Unable to find your indimailgroup group, specify --enable-indimailgroup.])
	fi
	])
AC_DEFINE_UNQUOTED(INDIMAILGROUP, "$indimailgroup", "indimail group name")
AC_SUBST(indimailgroup)

maxusersperpage=15
AC_ARG_ENABLE(maxusersperpage, [  --enable-maxusersperpage=#    Number of users to display on a page [15].], maxusersperpage="$enableval",)
AC_DEFINE_UNQUOTED(MAXUSERSPERPAGE, $maxusersperpage, "Number of users to displa on a page")

maxaliasesperpage=25
AC_ARG_ENABLE(maxaliasesperpage, [  --enable-maxaliasesperpage=#  Number of aliases to display on a page [25].], maxaliasesperpage="$enableval",)
AC_DEFINE_UNQUOTED(MAXALIASESPERPAGE, $maxaliasesperpage, "Number of aliases to display on a page")

NO_CACHE=no
AC_ARG_ENABLE(no-cache, [  --enable-no-cache   Prohibit caching via http headers.], NO_CACHE="$enableval",)

case $NO_CACHE in
	1*|y*|Y*)
	NO_CACHE=1
	AC_DEFINE_UNQUOTED(NO_CACHE, $NO_CACHE, "No Cache enabled or not")
	echo "checking whether we should prohibit caching... yes"
	;;
	*)
	echo "checking whether we should prohibit caching... no"
	;;
esac

IPAUTH=yes
AC_ARG_ENABLE(ipauth, [  --disable-ipauth    Disable IP address checks (NAT/proxy systems may not use the same IP for the whole session).],
	IPAUTH="$enableval",)

case $IPAUTH in
	1*|y*|Y*)
	IPAUTH=1
	AC_DEFINE_UNQUOTED(IPAUTH, $IPAUTH, "IP address check")
	echo "checking whether to require ip authentication... yes"
	;;
	*)
	echo "checking whether to require ip authentication... no"
	;;
esac

USER_INDEX=yes
AC_ARG_ENABLE(user-index, [  --disable-user-index   Don't display user index on show users page.], USER_INDEX="$enableval",)
case $USER_INDEX in
1*|y*|Y*)
	USER_INDEX=1
	AC_DEFINE_UNQUOTED(USER_INDEX, $USER_INDEX, "display user index on show users page or not")
	echo "checking whether to display user index... yes"
	;;
	*)
	echo "checking whether to display user index... no"
	;;
esac

AC_ARG_ENABLE(modify-quota, [  --enable-modify-quota  Allow domain admin (postmaster) to modify user quotas.],
	MODIFY_QUOTA="$enableval",)

case $MODIFY_QUOTA in
1*|y*|Y*)
	MODIFY_QUOTA=yes
	AC_DEFINE_UNQUOTED(MODIFY_QUOTA, 1, "Allow domain admin (postmaster) to modify user quotas")
	echo "checking whether to allow admins to modify user quotas... yes"
	;;
	*)
	MODIFY_QUOTA=no
	echo "checking whether to allow admins to modify user quotas... no"
	;;
esac

AC_ARG_ENABLE(domain-autofill, [  --enable-domain-autofill  Autofill the domain on login page based on the hostname that accesses iwebadmin.],
	DOMAIN_AUTOFILL="$enableval",)

case $DOMAIN_AUTOFILL in
1*|y*|Y*)
	DOMAIN_AUTOFILL=yes
	AC_DEFINE_UNQUOTED(DOMAIN_AUTOFILL, 1, "autofill domain on login page")
	echo "checking whether to autofill domain name on login page... yes"
	;;
	*)
	DOMAIN_AUTOFILL=no
	echo "checking whether to autofill domain name on login page... no"
	;;
esac


CATCHALL_ENABLED=yes
AC_ARG_ENABLE(catchall, [  --disable-catchall  Disable the use of catchall accounts.], CATCHALL_ENABLED="$enableval",)

case $CATCHALL_ENABLED in
1*|y*|Y*)
	CATCHALL_ENABLED=yes
	AC_DEFINE_UNQUOTED(CATCHALL_ENABLED,1,"")
	echo "checking whether to enable catchall accounts... yes"
	;;
	*)
	CATCHALL_ENABLED=no
	echo "checking whether to enable catchall accounts... no"
	;;
esac


TRIVIAL_PASSWORD_ENABLED=yes
AC_ARG_ENABLE(trivial-password, [  --disable-trivial-password  Disable the use of trivial password.], TRIVIAL_PASSWORD_ENABLED="$enableval",)

case $TRIVIAL_PASSWORD_ENABLED in
1*|y*|Y*)
	TRIVIAL_PASSWORD_ENABLED=yes
	AC_DEFINE_UNQUOTED(TRIVIAL_PASSWORD_ENABLED,1,"")
	echo "checking whether to enable trivial password... yes"
	;;
	*)
	TRIVIAL_PASSWORD_ENABLED=no
	echo "checking whether to enable trivial password... no"
	;;
esac

HELP=no
AC_ARG_ENABLE(help, [  --enable-help  Display help links on login page.], HELP="$enableval",)

case $HELP in
1*|y*|Y*)
	enable_help="yes"
	AC_SUBST(enable_help)
	AC_DEFINE_UNQUOTED(HELP,1,"")
	echo "checking whether to show help pages... yes"
	;;
	*)
	enable_help="no"
	echo "checking whether to show help pages... no"
	;;
esac
AC_DEFINE_UNQUOTED(PREFIX, "$prefix", "")

echo ""
echo "            $PACKAGE $VERSION"
echo "            Current settings"
echo "---------------------------------------"
echo "            prefix = "$prefix""
echo "       cgi-bin dir = "$cgibindir""
echo "          html dir = "$htmldir""
echo "         image dir = "$imagedir""
echo "         image URL = "$imageurl""
echo "      template dir = "$htmllibdir""
echo "         ezmlm dir = "$ezmlmdir""
echo "   ezmlm for MySQL = "$ezmlm_mysql""
echo "              help = "$enable_help""
echo "            libdir = "$libdir""
echo "      modify quota = "$MODIFY_QUOTA""
echo "   domain autofill = "$DOMAIN_AUTOFILL""
echo "          No Cache = "$NO_CACHE""
echo "           IP Auth = "$IPAUTH""
echo "        User Index = "$USER_INDEX""
echo "  Trivial Password = "$TRIVIAL_PASSWORD_ENABLED""
echo " catchall accounts = "$CATCHALL_ENABLED""

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([iwebadmin-release:iwebadmin-release.in],[], [])
AC_OUTPUT
