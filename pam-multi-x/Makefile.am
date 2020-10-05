@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c

DEFS=@DEFS@ -fno-strict-aliasing -I@prefix@/include/qmail

pamsbindir=@prefix@/sbin
pam_multidir=$(moduledir)
noinst_FILES=pam-multi.la pam-multi.a

pam_multi_LTLIBRARIES=pam-multi.la
pam_multi_la_SOURCES=pam-multi.c md5_crypt.c sha256_crypt.c sha512_crypt.c load_mysql.c
pam_multi_la_LDFLAGS=-module -avoid-version -version-info @LIBVER@:0:0
pam_multi_la_LIBADD=@LIB_MYSQL@ @LIB_QMAIL@ @LIB_PAM@ @LIB_QMAIL@ @LIB_CRYPT@ @LIB_DL@
pam_multi_la_CPPFLAGS=$(DEFS) -I@mysql_incdir@ -I@mysql_incdir@/mysql

pamsbin_PROGRAMS = pam-checkpwd$(EXEEXT)

man_MANS=pam-multi.8

all-local: pam-multi.spec imap pop3 pam-multi authcheckpassword

pam-checkpwd$(EXEEXT): pam-checkpwd.o pam-support.o 
	$(LINK) $@.o pam-support.o @LIB_PAM@
pam-checkpwd.o: pam-checkpwd.c pam-support.h config.h
	$(COMPILE) $(DEFS) -I@mysql_incdir@ -I@mysql_incdir@/mysql -c pam-checkpwd.c
pam-support.o: config.h pam-support.c

MOSTLYCLEANFILES=pam-multi.la pam-checkpwd pam-support.h.gch config.h.gch pam-multi.8 pam-multi \
	pop3 imap
EXTRA_DIST=COPYING NEWS README-pam-multi.md CREDITS ChangeLog INSTALL.pam-multi pam-multi.spec.in \
		   install-sh missing mkinstalldirs pam-multi.spec
DISTCLEANFILES=pam-multi.settings
doc_list = COPYING-pam-multi AUTHORS-pam-multi README-pam-multi.md

edit = sed \
	-e 's}@pam_confdir\@}$(pam_confdir)}g' \
	-e 's}@mandir\@}$(mandir)}g' \
	-e 's}@mysqlbindir\@}$(mysqlbindir)}g' \
	-e 's}@moduledir\@}$(moduledir)}g' \
	-e 's}@libdir\@}$(libdir)}g' \
	-e 's}@mysql_prefix\@}$(mysql_prefix)}g' \
	-e 's}@mysql_incdir\@}$(mysql_incdir)}g' \
	-e 's}@mysql_libdir\@}$(mysql_libdir)}g' \
	-e 's}@defaultdomain\@}$(defaultdomain)}g' \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@DESTDIR\@}$(DESTDIR)}g' \
	-e 's}@HOST\@}$(host)}g' \
	-e 's}@DATE\@}$(DATE)}g' \
	-e 's}@prefix\@}$(prefix)}g'

imap: imap.in
	([ -f imap.$(SYSTEM) ] && \
	$(edit) imap.in imap.$(SYSTEM) || $(edit) imap.in) > imap
pop3: pop3.in
	([ -f pop3.$(SYSTEM) ] && \
	$(edit) pop3.in pop3.$(SYSTEM) || $(edit) pop3.in) > pop3
pam-multi: pam-multi.in
	([ -f pam-multi.$(SYSTEM) ] && \
	$(edit) pam-multi.in pam-multi.$(SYSTEM) || $(edit) pam-multi.in) > pam-multi

authcheckpassword: authcheckpassword.in
	$(edit) $@.in > $@

pam-multi.8: pam-multi.9
	$(edit) pam-multi.9 > $@

install-data-local:  pam-multi.8 pam-checkpwd.8 pam-multi pop3 imap
	$(mkinstalldirs)  $(DESTDIR)@datarootdir@/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
	done
	$(mkinstalldirs)  $(DESTDIR)$(moduledir)
	$(mkinstalldirs)  $(DESTDIR)$(libdir)
	$(mkinstalldirs)  $(DESTDIR)$(pam_confdir)
	$(mkinstalldirs)  $(DESTDIR)@mandir@/man8
	$(INSTALL_DATA) -c pam-multi $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c pop3 $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c imap $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c pam-multi.8 $(DESTDIR)@mandir@/man8
	$(INSTALL_DATA) -c pam-checkpwd.8 $(DESTDIR)@mandir@/man8

uninstall-hook:
	/bin/rm -f $(DESTDIR)$(pam_confdir)/imap
	/bin/rm -f $(DESTDIR)$(pam_confdir)/pop3
	/bin/rm -f $(DESTDIR)$(pam_confdir)/pam-multi
