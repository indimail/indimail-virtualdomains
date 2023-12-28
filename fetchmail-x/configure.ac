dnl Autoconfigure input file for fetchmail
#
# Fetchmail automatic configuration support
#
# Eric S. Raymond <esr@thyrsus.com>
# 2004 - 2022 Matthias Andree <matthias.andree@gmx.de>
#
dnl Process this file with autoconf to produce a configure script.
dnl

dnl XXX - if bumping version here, check fetchmail.man, too!
AC_INIT([fetchmail],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-access@indimail.org],fetchmail, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CONFIG_SRCDIR([fetchmail.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_LIBOBJ_DIR([.])

AC_CANONICAL_HOST

dnl automake options are in Makefile.am
AC_PREREQ([2.67])
dnl 2.60 required for AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([silent-rules -Wall])
AM_SILENT_RULES

dnl python is optional
#
# you can pass PYTHON=: in environment or on the command line
# to disable python detection and continue without building/installing
# fetchmail.conf -- be sure to check README.packaging, too!
#
AM_PATH_PYTHON(2.7,,AC_MSG_WARN([Disabling fetchmailconf: python 2.7 or greater not found]))
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_CPP			dnl Later checks need this.
AC_PROG_CPP_WERROR
AM_PROG_AR
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

case "$ac_prog_cc_stdc" in
no|c89)
    echo "$CC $CPPFLAGS $CFLAGS"
    case "$CC $CPPFLAGS $CFLAGS" in
	*c++*|*g++*) : ;;
        *)        AC_MSG_ERROR([fetchmail requires a C99 capable compiler!])
        ;;
    esac
    ;;
esac

AC_CHECK_PROGS([RST2HTML], [rst2html5.py rst2html5], [false])
AC_CHECK_PROGS([HTMLDOC],  [htmldoc], [${srcdir}/dist-tools/htmldoc])

AC_LIB_RPATH

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

AC_C_RESTRICT

AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_TYPE_UINT32_T
AC_CHECK_HEADERS([sys/itimer.h sys/fcntl.h arpa/nameser.h langinfo.h])

AC_CHECK_HEADERS([resolv.h],,,[
#include <sys/types.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
])

AC_CHECK_DECLS([h_errno],,,[
	AC_INCLUDES_DEFAULT
	#include <netdb.h>
])

AM_MISSING_PROG([LEX], [flex])
AC_PROG_LEX([noyywrap])
case "$ac_cv_prog_LEX" in
	*flex*) ;; # we like that
	*) AC_MSG_WARN([Fetchmail requires flex to rebuild rcfile_l.*!]) ;;
esac
AC_PROG_MAKE_SET
AC_PROG_YACC
case "$ac_cv_prog_YACC" in
	*bison*) ;; # we like that
	*) AC_MSG_WARN([Fetchmail requires bison to rebuild rcfile_y.*!]) ;;
esac

# Check for OS special cases
case $host_os in
darwin*)
    AC_MSG_NOTICE(found Darwin - Adding -DBIND_8_COMPAT to CFLAGS)
    CPPFLAGS="$CPPFLAGS -DBIND_8_COMPAT"
    ;;
# Check for FreeBSD special case: more libs needed
freebsd*)
    AC_MSG_NOTICE(found FreeBSD - Adding -lkvm -lcom_err to standard libraries)
    LIBS="$LIBS -lkvm -lcom_err"
    ;;
esac

AC_CACHE_SAVE

dnl i18n
AM_GNU_GETTEXT([external], [need-ngettext])
AM_GNU_GETTEXT_VERSION([0.19.8])
dnl end i18n

# Under sysV68, socket and friends are provided by the C library.
# -linet does not provide socket, but causes multiple definition
# errors at link-time.  It is thus better to only use the C library.
# So do not add -linet to the link list unless it is necessary
AC_CHECK_FUNC(socket,
    AC_MSG_RESULT(using libc's socket),
    AC_CHECK_LIB(socket,socket)
    AC_CHECK_LIB(inet,socket))

AC_REPLACE_FUNCS([stpcpy strlcpy strlcat])

AC_CHECK_FUNC(getopt_long, [],
              [AC_LIBSOURCES([getopt.c, getopt1.c])
	       EXTRAOBJ="$EXTRAOBJ getopt.\$(OBJEXT) getopt1.\$(OBJEXT)"])

AC_SUBST(EXTRAOBJ)

AC_CHECK_FUNCS(vsyslog inet_aton strsignal)
dnl vsyslog, inet_aton: not in POSIX
dnl strsignal: only in POSIX 2008 (not in 2001)

# Under Red Hat 4.0 (and many other Linuxes) -lresolv is seriously flaky
# and breaks gethostbyname(2).  It's better to use the bind stuff in the C
# library.  So do not add -lresolv to the link list unless it is necessary
# (It will be necessary when using GNU libc6).
old_LIBS="$LIBS"
for lib in '' -lresolv; do
    if test -z "$lib"; then
       AC_MSG_CHECKING([for res_search in libc])
    else
       AC_MSG_CHECKING([for res_search in $lib])
    fi
    LIBS="$old_LIBS $lib"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif
extern int res_search();
]], [[res_search(0, 0, 0, 0, 0); dn_skipname(0,0);]])],
    [AC_MSG_RESULT([found])
     AC_DEFINE(HAVE_RES_SEARCH, [1],
	       [Define to 1 if you have the 'res_search' and 'dn_skipname' functions.])
     break], [AC_MSG_RESULT([not found])])
     LIBS=$old_LIBS
done

AC_MSG_CHECKING(sys_siglist declaration in signal.h or unistd.h)
AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[
	    #include <signal.h>
/* NetBSD declares sys_siglist in <unistd.h>.  */
#include <unistd.h>
	]], [[
	    char *msg = *(sys_siglist + 1);
	]]
	)
],
[AC_DEFINE(SYS_SIGLIST_DECLARED,1,[Define if 'sys_siglist' is declared by <signal.h>.])
 AC_MSG_RESULT(yes)],
[AC_MSG_RESULT(no)])

# Find the right directory to put the root-mode PID file in
for dir in "/var/run" "/etc"
do
	if test -d $dir
	then
		break;
	fi
done
AC_MSG_RESULT(root-mode pid file will go in $dir)
AC_DEFINE_UNQUOTED(PID_DIR, "$dir", directory for PID lock files)

AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

###	use option --enable-POP2 to compile in the POP2 support
AC_ARG_ENABLE(POP2,
	[  --enable-POP2           compile in POP2 protocol support (obsolete)],
	[with_POP2=$enableval],
	[with_POP2=no])
test "$with_POP2" = "yes" && AC_DEFINE(POP2_ENABLE,1,Define if you want POP2 support compiled in)
AM_CONDITIONAL(POP2_ENABLE, test "$with_POP2" = yes)

###	use option --disable-POP3 to omit the POP3 support
AC_ARG_ENABLE(POP3,
	[AS_HELP_STRING([--disable-POP3],
			[do not compile in POP3 protocol support])],
	[with_POP3=$enableval],
	[with_POP3=yes])
test "$with_POP3" = "yes" && AC_DEFINE(POP3_ENABLE,1,Define if you want POP3 support compiled in)
AM_CONDITIONAL(POP3_ENABLE, test "$with_POP3" = yes)

###	use option --disable-IMAP to omit the IMAP support
AC_ARG_ENABLE(IMAP,
	[AS_HELP_STRING([--disable-IMAP],
			[do not compile in IMAP protocol support])],
	[with_IMAP=$enableval],
	[with_IMAP=yes])
test "$with_IMAP" = "yes" && AC_DEFINE(IMAP_ENABLE,1,Define if you want IMAP support compiled in)
AM_CONDITIONAL(IMAP_ENABLE, test "$with_IMAP" = yes)

###	use option --disable-ETRN to omit the ETRN support
AC_ARG_ENABLE(ETRN,
	[AS_HELP_STRING([--disable-ETRN],[do not compile in ETRN protocol
	 support])],
	[with_ETRN=$enableval],
	[with_ETRN=yes])
test "$with_ETRN" = "yes" && AC_DEFINE(ETRN_ENABLE,1,Define if you want ETRN support compiled in.)
AM_CONDITIONAL(ETRN_ENABLE, test "$with_ETRN" = yes)

###	use option --disable-ODMR to omit the ODMR support
AC_ARG_ENABLE(ODMR,
	[AS_HELP_STRING([--disable-ODMR],[do not compile in ODMR protocol
	 support])],
	[with_ODMR=$enableval],
	[with_ODMR=yes])
test "$with_ODMR" = "yes" && AC_DEFINE(ODMR_ENABLE,1,Define if you want ODMR support compiled in)
AM_CONDITIONAL(ODMR_ENABLE, test "$with_ODMR" = yes)

###	use option --enable-RPA to compile in the RPA support
AC_ARG_ENABLE(RPA,
	[AS_HELP_STRING([--enable-RPA],[compile in RPA protocol
	 support])],
	[with_RPA=$enableval],
	[with_RPA=no])
test "$with_RPA" = "yes" && AC_DEFINE(RPA_ENABLE,1,Define if you want RPA support compiled in)
AM_CONDITIONAL(RPA_ENABLE, test "$with_RPA" = yes)

###	use option --enable-indimail to compile in the INDIMAIL support
AC_ARG_ENABLE(indimail,
	[  --enable-indimail            compile in INDIMAIL protocol support],
	[with_INDIMAIL=$enableval],
	[with_INDIMAIL=no])
test "$with_INDIMAIL" = "yes" && AC_DEFINE(INDIMAIL,1,Define if you want INDIMAIL support compiled in)
AM_CONDITIONAL(INDIMAIL, test "$with_INDIMAIL" = yes)

###	use option --enable-NTLM to compile in the NTLM support
AC_ARG_ENABLE(NTLM,
	[AS_HELP_STRING([--enable-NTLM],[compile in NTLM authentication
	 support])],
	[with_NTLM=$enableval],
	[with_NTLM=no])
test "$with_NTLM" = "yes" && AC_DEFINE(NTLM_ENABLE,1,Define if you want NTLM authentication)
AM_CONDITIONAL(NTLM_ENABLE, test "$with_NTLM" = yes)

###	use option --enable-SDPS to compile in the SDPS support
AC_ARG_ENABLE(SDPS,
	[AS_HELP_STRING([--enable-SDPS],[compile in SDPS protocol support])],
	[with_SDPS=$enableval],
	[with_SDPS=no])
if test "$with_SDPS" = yes ; then
   if test "$with_POP3" != yes ; then
       AC_MSG_WARN([SDPS cannot be enabled with POP3 disabled. Disabling SDPS.])
       with_SDPS=no
   else
       AC_DEFINE(SDPS_ENABLE,1,Define if you want SDPS support compiled in)
   fi
fi
if test "$with_POP3" != yes && test "$with_POP2" != yes \
    && test "$with_IMAP"  != yes ; then
    AC_MSG_ERROR([You must enable at least one of POP2, POP3 and IMAP.])
fi

AC_CACHE_SAVE

###	use option --enable-opie to compile in the OPIE support
AC_ARG_ENABLE(opie,
	[AS_HELP_STRING([--enable-opie],[support one-time passwords
	 (OTP) through the OPIE library])],
	[ AC_CHECK_HEADER(opie.h,, [AC_MSG_ERROR([cannot find <opie.h>, which is required for OPIE support.])])
          AC_CHECK_LIB(opie,opiegenerator,, [AC_MSG_ERROR([cannot find libopie, which is required for OPIE support.])])
          with_opie=$enableval],
	[with_opie=no])
test "$with_opie" = "yes" && AC_DEFINE(OPIE_ENABLE,1,Define if you want OPIE support compiled in)

dnl Check if getaddrinfo is async-signal-safe - most implementations aren't

    AC_MSG_CHECKING(if your getaddrinfo is async-signal-safe)
    gai_ts=no
dnl we have getaddrinfo() - check if the OS is known to have a async-signal-safe/thread-safe implementation
    case $host_os in
    darwin*)				gai_ts=yes ;;
    linux*)				gai_ts=yes ;;
    freebsd*)				gai_ts=yes ;;
    solaris*)				gai_ts=yes ;;
    esac
    AC_MSG_RESULT($gai_ts)
    if test $gai_ts = yes ; then
	AC_DEFINE(GETADDRINFO_ASYNCSAFE, 1, [define to 1 if you know your getaddrinfo function is async-signal-safe])
    fi

# This version of the Kerberos 4 and 5 options addresses the follwing issues:
#
# * Build correctly under Heimdal kerberos if it is compiled with db2 and
#   OpenSSL support (Debian's is)
# * Build the kerberos.c stuff (KPOP) only for kerberosIV, to avoid breakage.
#   I don't know if this is 100% correct, but now at least IMAP and POP3
#   behave the same way regarding kerberosV.
# * Build without any fuss for both kerberosIV and V at the same time.
# * Move all the kerberos header mess to kerberos.h, and #include that
#   in driver.c and kerberos.c.
#
# Tested using the Heimdal Kerberos V libs, Kungliga Tekniska Högskolan (the
# Royal Institute of Technology in Stockholm, Sweden)'s kerberos IV libs, and
# the MIT reference implementation of KerberosV (all as packaged in Debian).

###    use option --with-kerberos5=DIR to point at a Kerberos 5 directory
### make sure --with-ssl is run before --with-kerberos* !
AC_ARG_WITH(kerberos5,
       [AS_HELP_STRING([--with-kerberos5[[=DIR]]],[point fetchmail
	compilation at a Kerberos 5 directory])],
[
if test "$with_kerberos5" != "no"
then
# Check for a OpenBSD special case
if test "$with_kerberos5" = "yes" && ( test `uname` = "OpenBSD" )
then
  AS_MESSAGE(checking kerberosV for OpenBSD...)
  AC_DEFINE(HEIMDAL,1,Define if you have HEIMDAL kerberos 5)
  AC_DEFINE(KERBEROS_V5,1,Define if you have Kerberos V5)
  CFLAGS="$CFLAGS -I/usr/include/kerberosV"
  LIBS="$LIBS -lasn1 -lkrb5 -lcom_err -lkafs"
elif krb5-config 2> /dev/null >/dev/null ; then
  krb5_prefix=`krb5-config --prefix krb5`
  AC_MSG_RESULT([krb5-config points to kerberosV under $krb5_prefix])
  if test -f ${krb5_prefix}/include/et/com_err.h && \
   ! test -f ${krb5_prefix}/include/com_err.h  ; then
    CFLAGS="$CFLAGS -I${krb5_prefix}/include/et"
  fi
  unset krb5_prefix
  CFLAGS="$CFLAGS `krb5-config --cflags krb5`"
  LIBS="$LIBS `krb5-config --libs krb5`"
  AC_DEFINE(KERBEROS_V5)
else
  if test "$with_kerberos5" != "yes"
  then
      LDFLAGS="$LDFLAGS -L${with_kerberos5}/lib"
      searchdirs="$with_kerberos5"
  else
      searchdirs="/usr/kerberos /usr/local/krb5 /usr/athena /usr"
  fi
  with_kerberos5=
  for dir in $searchdirs
  do AC_MSG_CHECKING([for Kerberos V in $dir])
     if test -f "$dir/include/krb5.h" || test -f "$dir/include/krb5/krb5.h"
     then
	if test -d "$dir/include/krb5" ; then CPPFLAGS="$CPPFLAGS -I$dir/include/krb5" ; fi
        ac_krblibs=
        if test -f "$dir/include/roken.h" || test -f "$dir/include/krb5/roken.h"
        then
           ac_krblibs="-lasn1 -lroken -lcom_err"
           AC_MSG_RESULT([Heimdal found])
          dnl Attempt to detect if we need to -ldb2 to link Heimdal
          dnl we assume we do if it is available
           AC_CHECK_LIB(db2, db_open, ac_krblibs="$ac_krblibs -ldb2", [],
                ${LDFLAGS})
           AC_CHECK_LIB(des, des_string_to_key, libk5crypto=-ldes,
              AC_CHECK_LIB(crypto, MD5_Init, [],
                AC_MSG_ERROR([DES libraries not found. Try adding --with-ssl to enable OpenSSL support]),
                ${LDFLAGS} ${ac_krblibs}),
           ${LDFLAGS} ${ac_krblibs})
           AC_DEFINE(HEIMDAL)
        else
          AC_MSG_RESULT([found])
          ac_krblibs="-lcom_err"
           AC_CHECK_LIB(crypto,
             krb5_des_string_to_key,
             libk5crypto=-lcrypto,
                AC_CHECK_LIB(k5crypto,
                   krb5_des_string_to_key,
                   libk5crypto=-lk5crypto,
                      AC_MSG_ERROR([Kerberos 5 DES libraries not found]),
                   ${LDFLAGS} ${ac_krblibs}),
                 ${LDFLAGS} ${ac_krblibs})
        fi
        AC_CHECK_LIB(krb5, krb5_init_context,[],
           continue,
           ${LDFLAGS} ${ac_krblibs})
        AC_DEFINE(KERBEROS_V5)
        test "$dir" != "/usr" && CFLAGS="$CFLAGS -I$dir/include"
        LDFLAGS="$LDFLAGS -L$dir/lib"
        LIBS="$LIBS $ac_krblibs $libk5crypto"
        with_kerberos5=$dir
        break
     else
        AC_MSG_RESULT([not found])
     fi
  done
  if test -z "$with_kerberos5" ; then
     AC_MSG_ERROR([Kerberos 5 libraries not found])
  fi
fi
fi
]) dnl --with-kerberos5=DIR

###    use option --with-kerberos=DIR to point at a Kerberos 4 directory
KERBEROS_V4=0
AC_ARG_WITH(kerberos,
       [  --with-kerberos=DIR     point fetchmail compilation at a Kerberos 4 directory],
[
if test "$with_kerberos" != "no"
then
    AC_MSG_WARN([Kerberos IV support is obsolete. Use --with-kerberos5 if possible.])
# Check for a NetBSD/OpenBSD special case
if test "$with_kerberos" = "yes" && ( test `uname` = "NetBSD" || test `uname` = "OpenBSD" )
then
  AS_MESSAGE(checking kerberosIV for `uname`...)
  KERBEROS_V4=1
  CFLAGS="$CFLAGS -I/usr/include/kerberosIV"
  case `uname` in
      NetBSD)  LIBS="$LIBS -lkrb -ldes -lroken -lcom_err" ;;
      OpenBSD) LIBS="$LIBS -lkrb -ldes" ;;
  esac
elif krb4-config 2> /dev/null >/dev/null ; then
  krb4_prefix=`krb4-config --prefix`
  AC_MSG_RESULT([krb4-config points to kerberosIV under $krb4_prefix])
  unset krb4_prefix
  CFLAGS="$CFLAGS `krb4-config --cflags`"
  LIBS="$LIBS `krb4-config --libs`"
  KERBEROS_V4=1
elif krb5-config 2> /dev/null >/dev/null ; then
  krb4_prefix=`krb5-config --prefix krb4`
  AC_MSG_RESULT([krb5-config points to kerberosIV under $krb4_prefix])
  if test -f ${krb4_prefix}/include/kerberosIV/krb.h ; then
    AC_DEFINE(KERBEROS_V4_V5,1,Define if you have Kerberos V4 headers under a kerberosIV directory)
  fi
  unset krb4_prefix
  CFLAGS="$CFLAGS `krb5-config --cflags krb4`"
  LIBS="$LIBS `krb5-config --libs krb4`"
  KERBEROS_V4=1
else
  #we need to detect when we're building under a kerberosV compatibility
  #layer, btw...
  if test "$with_kerberos" != "yes" ; then
     searchdirs="$with_kerberos"
  else
     searchdirs="$with_kerberos5 /usr/kerberos /usr/kerberosIV /usr/athena /usr"
  fi
  with_kerberos=
  ac_saveLDFLAGS="$LDFLAGS"
  for dir in $searchdirs
  do
     AC_MSG_CHECKING([for Kerberos IV in $dir])
     if test -f "$dir/include/krb.h" || test -f "$dir/include/krb4.h" \
            || test -f "$dir/include/kerberosIV/krb.h"
     then
        AC_MSG_RESULT([found])
     else
        AC_MSG_RESULT([not found])
       continue
     fi
     #Find libs
     if test -f "$with_kerberos5/roken.h" ; then
       AC_CHECK_LIB(45, krb_mk_req)
     fi
     LDFLAGS="-L$dir/lib $ac_saveLDFLAGS"
     if test `uname` = "FreeBSD"; then
        AC_SEARCH_LIBS(_ossl_old_des_string_to_key, [des425 des crypto], [], continue)
     else
        AC_SEARCH_LIBS(des_string_to_key, [crypto], [], continue)
     fi
     AC_SEARCH_LIBS(krb_realmofhost, [krb4 krb], [], continue)
     with_kerberos="$dir"
     if test -f "$dir/include/kerberosIV/krb.h" ; then
        dir="$dir/include/kerberosIV"
     else
       dir="$dir/include"
     fi
     KERBEROS_V4=1
     test -f "$with_kerberos5/roken.h" && AC_DEFINE(HEIMDAL)
     test "$dir" != "/usr/include" && CFLAGS="$CFLAGS -I$dir"
     LDFLAGS="$LDFLAGS -L$with_kerberos/lib"
     break
  done
  if test -z "$with_kerberos" ; then
     AC_MSG_ERROR([Kerberos 4 libraries not found])
  fi
  LDFLAGS="$ac_saveLDFLAGS"
fi
fi
]) dnl --with-kerberos=DIR
if test "$KERBEROS_V4" = 1 ; then
    AC_DEFINE(KERBEROS_V4,1,Define if you have Kerberos V4)
fi
AM_CONDITIONAL(KERBEROS_V4_ENABLE, test "$KERBEROS_V4" = 1)

openssldefault=yes
AC_ARG_WITH(wolfssl,
	    [AS_HELP_STRING([--with-wolfssl={DIR|no}],
			    [EXPERIMENTAL: Use wolfSSL and its OpenSSL compatibility layer])],
		    [with_wolfssl=$withval], [with_wolfssl=no])
if test "$with_wolfssl" != "no" ; then
	if test -x "$WOLFSSL_CONFIG" ; then
		_WOLFSSLCONF="$WOLFSSL_CONFIG"
	elif test yes != "$with_wolfssl" -a -x "${with_wolfssl}/bin/wolfssl-config" ; then
		_WOLFSSLCONF=${with_wolfssl}/bin/wolfssl-config
	else
		_WOLFSSLCONF=wolfssl-config
	fi
	if test yes = "$with_wolfssl" && "$_WOLFSSLCONF" >/dev/null 2>&1 --version ; then
		with_wolfssl="$(${_WOLFSSLCONF} --prefix)"
	fi
	if "$_WOLFSSLCONF" >/dev/null 2>&1 --version ; then
		wolfCFLAGS="$(${_WOLFSSLCONF} --cflags)"
		wolfLIBS="$(${_WOLFSSLCONF} --libs)"
	elif test -d "${with_wolfssl}/include" -a -d "${with_wolfssl}/lib" ; then
		wolfCFLAGS="-I${with_wolfssl}/include"
		wolfLIBS="-L${with_wolfssl}/lib -lwolfssl -lpthread"
		AS_MESSAGE([wolfssl-config not found nor executable, falling back to CFLAGS $wolfCFLAGS, LIBS $wolfLIBS])
	else
		AC_MSG_ERROR([WolfSSL not found - please specify proper location to --with-wolfssl])
	fi
	set -- $wolfLIBS
	while test $# -ge 1 ; do
		case $1 in -l*|lib*) :                     ;;
				  *) LDFLAGS="$LDFLAGS $1" ;;
		esac
		shift
	done
	CPPFLAGS="$CPPFLAGS $wolfCFLAGS -I${with_wolfssl}/include/wolfssl"
	AC_CHECK_HEADERS([wolfssl/options.h])
	AC_LIB_LINKFLAGS([wolfssl])
	LIBS="$LIBWOLFSSL $LIBS"
	openssldefault=no
	AC_DEFINE(SSL_ENABLE, 1)
	with_ssl=yes
	if test -z "$WOLFSSL_TRUST_FILE" ; then
		AC_MSG_ERROR([You must define WOLFSSL_TRUST_FILE and point it to the default CA certificate file (PEM format)])
	fi
	AS_MESSAGE([Enabling wolfSSL support.])
	AC_DEFINE_UNQUOTED(WOLFSSL_TRUST_FILE, "$WOLFSSL_TRUST_FILE",
			   [Set to the default file of trusted certificates.])
else
  ###	use option --with-ssl to compile in the SSL support
  AC_ARG_WITH(ssl,
	  [AS_HELP_STRING([--with-ssl[[={yes|no|DIR}]]],[Use OpenSSL from
	   DIR/include/openssl and
	   DIR/lib (default: yes). Yes means try to find it.])],
	  [with_ssl=$withval],
	  [with_ssl=$openssldefault])

  if test "$with_ssl" = "yes"
  then
      #	User didn't specify an SSL location.  Let's look at some common
      #	directories where SSL has been found in the past and try and auto
      #	configure for SSL.  OpenSSL determination will be made later.
      #	This will screw up if an OpenSSL install is located in a later
      #	directory than an older SSLeay install, but the user should fix that
      #	anyways and he can override on the configure line.
      #	Just testing for directories is not sufficient, /usr exists on
      #	all systems!
      for ac_dir in \
	/usr/local/ssl \
	/usr/local \
	/usr/ssl \
	/usr \
	/local/ssl \
	/opt/ssl \
	/opt/csw \
	; \
      do
	  if test -r "$ac_dir/include/openssl/ssl.h" ; then
	      with_ssl=$ac_dir
	      break;
	  fi
      done
  fi

  if test -n "$with_ssl" -a "$with_ssl" != "no"
  then
    # With the autoconfigure above, the only time this is going to be
    # true is going to be when we could not find the headers.  If they
    # are not in system standard locations, we are going to be broken.
    if test "$with_ssl" = "yes"
    then
      # Let's just define the standard location for the SSLeay root
      with_ssl="/usr/local/ssl"
    fi
    if test -r $with_ssl/include/openssl/ssl.h
    then
      ###	ssl.h found under openssl.  Use openssl configuration preferentially,
      AC_MSG_NOTICE(Enabling OpenSSL support in $with_ssl.)
      test "$with_ssl" != "/usr" && CFLAGS="$CFLAGS -I$with_ssl/include" LIBS="-L$with_ssl/lib $LIBS"
      ### In Red Hat 9, this file includes a reference to <krb5.h>, so we
      ### force the Kerberos directory onto the include path so it will build.
      CFLAGS="$CFLAGS -I/usr/kerberos/include"
      ###	OpenBSD comes with ssl headers
    else
      case "$with_ssl" in */*)
	      AC_MSG_ERROR([SSL support enabled, but OpenSSL not found])
	      ;;
	      *)
	      ;;
      esac
    fi
    modules=openssl
    case "$with_ssl" in */*) ;; *) modules="$with_ssl $modules" ;; esac
    found=0
    test -n "$PKG_CONFIG" && for i in $modules ; do
    AS_MESSAGE([SSL-check: trying pkg-config for $i])
    AS_MESSAGE([NOTE: libdir=$libdir])
    PKG_CHECK_MODULES([SSL],[$i],[
	  set -- $SSL_LIBS
	  while test $# -ge 1 ; do
		  case $1 in -l*|lib*) LIBS="$LIBS $1"       ;;
				    *) LDFLAGS="$LDFLAGS $1" ;;
		  esac
		  shift
	  done
	  CPPFLAGS="$SSL_CFLAGS $CPPFLAGS"
	  AS_MESSAGE([From pkg-config: $SSL_LIBS])
	  AC_LIB_LINKFLAGS_FROM_LIBS([SSL_LDFLAGS],[$LIBS $SSL_LIBS])
	  AS_MESSAGE([derived LDFLAGS: $SSL_LDFLAGS])
	  LDFLAGS="$SSL_LDFLAGS $LDFLAGS"
	  found=1
	  break],[: ignore-error])
    done
    if test $found -eq 0 ; then
      AS_MESSAGE([SSL-check: pkg-config check failed, using traditional probe in $with_ssl])
      LDFLAGS="$LDFLAGS -L$with_ssl/lib"
      CPPFLAGS="$CPPFLAGS -I$with_ssl/include"
      AC_LIB_LINKFLAGS([ssl], [crypto])
      AS_MESSAGE([From probing: Adding $LIBSSL to LIBS. LDFLAGS=$LDFLAGS])
      LIBS="$LIBS $LIBSSL"
      dnl check if -ldl is needed
      AC_MSG_CHECKING([for additional library dependencies of SSL (-ldl?)])
      found=0
      save_LIBS="$LIBS"
      for i in "" "-ldl" ; do
	  LIBS="$LDFLAGS $save_LIBS $i"
	  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <openssl/ssl.h>],[SSL_connect((SSL *)0)])],[found=1; break])
      done
      if test $found = 0 ; then
	  AC_MSG_RESULT([error])
	  AC_MSG_ERROR([cannot link with SSL - check config.log])
      fi
      LIBS="$save_LIBS $i"
      if test "$i" = "" ; then i="(none)" ; fi
      AC_MSG_RESULT($i)
    fi
    AC_DEFINE(SSL_ENABLE, 1, [Define if you want SSL support compiled in])
    AS_MESSAGE([Enabling OpenSSL support.])
  else
    AC_MSG_WARN(Disabling SSL support.)
    AC_MSG_WARN(Consider re-running configure --with-ssl.)
  fi
fi

AS_MESSAGE([
  CC:       $CC
  CPPFLAGS: $CPPFLAGS
  CFLAGS:   $CFLAGS
  LDFLAGS:  $LDFLAGS
  LIBS:     $LIBS])

case "$LIBS" in *-lssl*|*libssl*|*-lwolfssl*|*libwolfssl*)
	case $host_os in
	openbsd*) ;; dnl OpenBSD ships LibreSSL in the base operating system
		     dnl so makes use of the GPL2 clause 3 exception.
	*)
		AC_CHECK_DECLS([LIBRESSL_VERSION_NUMBER],
		       AC_MSG_ERROR([fetchmail cannot legally be linked against LibreSSL for lack of GPL2 clause 2b exception. See COPYING.]),,
		       [#include <openssl/ssl.h>])
		;;
	esac
	AC_CHECK_DECLS([TLS1_3_VERSION],,
		       AC_MSG_WARN([Your OpenSSL version is too old and does not support TLS v1.3. Upgrade.]),
		       [#include <openssl/ssl.h>])
	AC_CHECK_DECLS([TLS1_2_VERSION],,
		       AC_MSG_ERROR([Your OpenSSL version is too old and does not support TLS v1.2. Upgrade.]),
		       [#include <openssl/ssl.h>])
	AC_CHECK_DECLS([SSLv3_client_method],,,[#include <openssl/ssl.h>])
	;;
esac

###	use option --with-socks=DIR to point at SOCKS library
AC_ARG_WITH(socks,
	[AS_HELP_STRING([--with-socks[[=DIR]]],[add built-in SOCKS (4)
	firewall access])],
[
if test "$with_socks" != no
then
    if test "$with_socks" != yes
    then
        LDFLAGS="$LDFLAGS -L$with_socks"
    else
        AC_CHECK_LIB(socks, Rconnect,,
               AC_MSG_ERROR([could not find libsocks which is needed for built-in SOCKS support]))
    fi
    AC_DEFINE(HAVE_SOCKS,1,Define if you want built-in SOCKS support)
    CFLAGS="$CFLAGS -Dconnect=Rconnect -Dgetsockname=Rgetsockname -Dbind=Rbind -Daccept=Raccept -Dlisten=Rlisten -Dselect=Rselect"
    LIBS="-lsocks $LIBS"
fi])

###     use option --with-socks5=DIR to point at SOCKS library
AC_ARG_WITH(socks5,
        [AS_HELP_STRING([--with-socks5[[=DIR]]],[add built-in SOCKS5
	 firewall access])],
[
if test "$with_socks5" != no
then
  if test "$with_socks5" != yes
  then
    LDFLAGS="$LDFLAGS -L$with_socks5"
  fi
  ac_savedLDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $LDFLAGS"
  AC_CHECK_LIB(socks5, SOCKSconnect,,
	AC_MSG_ERROR([could not find libsocks5 which is needed for built-in SOCKS5 support]))
  AC_DEFINE(HAVE_SOCKS)
  CFLAGS="$CFLAGS -Dconnect=SOCKSconnect -Dgetsockname=SOCKSgetsockname -Dbind=SOCKSbind -Daccept=SOCKSaccept -Dlisten=SOCKSlisten -Dselect=SOCKSselect -Drecvfrom=SOCKSrecvfrom -Dsendto=SOCKSsendto -Drecv=SOCKSrecv -Dsend=SOCKSsend -Dread=SOCKSread -Dwrite=SOCKSwrite -Drresvport=SOCKSrresvport -Dshutdown=SOCKSshutdown -Dlisten=SOCKSlisten -Dclose=SOCKSclose -Ddup=SOCKSdup -Ddup2=SOCKSdup2"
  AC_CHECK_FUNC(SOCKSfclose, [CFLAGS="$CFLAGS -Dfclose=SOCKSfclose"])
  LDFLAGS="$ac_savedLDFLAGS"
fi])

###	use option --with-hesiod=DIR to point at a HESIOD directory
AC_ma_SEARCH_PACKAGE(hesiod, hesiod_getmailhost, /usr/athena /usr /usr/local,hesiod, hesiod.h)

###	use option --with-gssapi=DIR to compile in GSSAPI support
AC_ARG_WITH(gssapi,
	[AS_HELP_STRING([--with-gssapi[[=DIR]]],[compile in GSSAPI
	 support using libraries in DIR])],
[
if test "$with_gssapi" != "no"
then
  if test "$with_gssapi" = "yes" -a -n "$with_kerberos5"
  then
    with_gssapi=$with_kerberos5
  fi

  if test "$with_gssapi" != "yes" -a -n "$with_gssapi"
  then
    CFLAGS="$CFLAGS -I$with_gssapi/include"
    LDFLAGS="$LDFLAGS -L$with_gssapi/lib"
  fi

  dnl obtain gssapi default libs
  if krb5-config --libs gssapi 2>/dev/null >/dev/null ; then
      LIBS="$LIBS `krb5-config --libs gssapi`"
  else
    AC_CHECK_LIB(gss, gss_check_version, LIBS="$LIBS -lgss", nogss=t)
    if test -n "$nogss"
    then
      # -lgss not found
      if test "$with_gssapi" = yes ; then
	  with_gssapi=/usr
      fi
      if test -f "$with_gssapi/include/roken.h" -o -f "$with_gssapi/include/krb5/roken.h"
      then
         # have roken.h, assume Heimdal
	 case `uname` in
	     NetBSD) LIBS="$LIBS -lkrb5" ;;
	     *)      LIBS="$LIBS -lkrb5 -lasn1 -ldes -lroken -ldb -ldl" ;;
	 esac
         AC_CHECK_LIB(gssapi, gss_init_sec_context,LIBS="$LIBS -lgssapi",
                      AC_MSG_ERROR([could not find libgssapi which is needed for GSSAPI support]), )
         AC_DEFINE(HEIMDAL)
      else
         # no roken.h, assume MIT
         AC_CHECK_LIB(gssapi_krb5, gss_init_sec_context,LIBS="$LIBS -lgssapi_krb5",
                      AC_MSG_ERROR([could not find libgssapi_krb5 which is needed for GSSAPI support]), -lkrb5)
      fi
    fi
  fi
  AC_DEFINE(GSSAPI,1,Define if you want GSSAPI authentication)
  if test "$with_gssapi" != yes ; then
    CPPFLAGS="$CPPFLAGS -I$with_gssapi/include"
  fi
  AC_CHECK_HEADERS(gss.h gssapi.h gssapi/gssapi.h gssapi/gssapi_generic.h)
  AC_CHECK_DECLS(GSS_C_NT_HOSTBASED_SERVICE,,,[
    AC_INCLUDES_DEFAULT
    #if HAVE_GSS_H
    #include <gss.h>
    #endif
    #if HAVE_GSSAPI_GSSAPI_H
    #include <gssapi/gssapi.h>
    #elif HAVE_GSSAPI_H
    #include <gssapi.h>
    #endif
    #if HAVE_GSSAPI_GSSAPI_GENERIC_H
    #include <gssapi/gssapi_generic.h>
    #endif
    ])
fi])

AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_OUTPUT

dnl Local Variables:
dnl comment-start: "dnl "
dnl comment-end: ""
dnl comment-start-skip: "\\bdnl\\b\\s *"
dnl compile-command: "make configure config.h.in"
dnl End:
