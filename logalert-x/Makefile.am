@SET_MAKE@
ACLOCAL_AMFLAGS = -I m4
version=@VERSION@
COFLAGS=-z+05:30
DEFS = @DEFS@ -fno-strict-aliasing -Wno-implicit-function-declaration -Wno-unused-function -Wno-return-type
INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c

bin_PROGRAMS=logclient incrmesg logalert showbytes
sbin_PROGRAMS=logsrv rpclog

man_MANS = logalert.8 logsrv.8 rpclog.8 incrmesg.1 logclient.1 showbytes.8
mandir = @prefix@/share/man

doc_DATA = LICENSE README.md
noinst_SCRIPTS = logalert.spec logalert-rpmlintrc logalert.changes PKGBUILD

MOSTLYCLEANFILES=lex.c parser.h parser.c logalert.perms debian/Makefile
DISTCLEANFILES = *~

SYSCONFDIR = @sysconfdir@
sysconf_DATA = logalert.perms logalert-release

AM_LFLAGS  = -Pc
AM_YFLAGS  = -d

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e "s,@release\@,"`cat conf-release`",g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@DESTDIR\@,$(DESTDIR),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@HOST\@,$(host),g' \
	-e 's,@DATE\@,$(DATE),g' \
	-e 's,@prefix\@,$(prefix),g'

logalert_SOURCES = lex.c utils.c log.c parser.y process.c monitor.c action.c sig.c user.c main.c 
logalert_LDADD = @LEXLIB@ @LIBPCRE_LIBS@

logclient_SOURCES=logclient.c
logclient_CPPFLAGS=-DSERVER -Wno-empty-body -I/usr/include/qmail
logclient_LDADD = $(LIB_SSL) $(LIB_QMAIL)

logsrv_SOURCES=logsrv.c
logsrv_CPPFLAGS=-Wno-empty-body -I/usr/include/qmail
logsrv_LDADD = $(LIB_SSL) $(LIB_QMAIL) $(LIB_RPC)

rpclog_LDADD=$(LIB_RPC)

incrmesg_SOURCES=incrmesg.c
incrmesg_CPPFLAGS=-Wno-empty-body -I/usr/include/qmail
incrmesg_LDADD = $(LIB_QMAIL)

showbytes_SOURCES=showbytes.c
showbytes_CPPFLAGS=-Wno-empty-body -I/usr/include/qmail
showbytes_LDADD = $(LIB_QMAIL)

#$(am__skiplex) $(SHELL) $(YLWRAP) $< $(LEX_OUTPUT_ROOT).c $@ -- flex
lex.c: lex.l parser.h
	 $(LEX) $(LFLAGS) -o $@ lex.l parser.h

parser.h: parser.c
logalert-rpmlintrc: logalert-rpmlintrc.in
	$(edit) logalert-rpmlintrc.in > $@
logalert.spec: logalert.spec.in doc/ChangeLog conf-version \
conf-release conf-email
	(cat $@.in;./catChangeLog --spec doc/ChangeLog) | $(edit)> $@
logalert.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release conf-email \
logalert.changes
	cat $@.in | $(edit) > $@
logsrv.8: logsrv.man
	$(edit) logsrv.man > $@
logclient.1: logclient.man
	$(edit) logclient.man > $@
debian/Makefile: debian/Makefile.in conf-version conf-release
	$(edit) $@.in > $@
logalert.perms: perm_list.in
	$(edit) perm_list.in > $@
install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)$(SYSCONFDIR)
	$(INSTALL_DATA) logalert.perms $(DESTDIR)$(SYSCONFDIR)/logalert.perms  || exit 1
