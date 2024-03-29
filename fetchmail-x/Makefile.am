# Makefile for fetchmail

SUBDIRS=		. po
AUTOMAKE_OPTIONS=	1.12 foreign no-dist-gzip dist-xz color-tests parallel-tests subdir-objects

.DELETE_ON_ERROR:	# yes please
.PRECIOUS:		test-suite.log

ACLOCAL_AMFLAGS=	-I m4 -I m4-local
AM_YFLAGS=		-t -d
BUILT_SOURCES=		rcfile_y.h rcfile_l.h

bin_PROGRAMS=		fetchmail

dist_noinst_SCRIPTS=	specgen.sh
dist_man1_MANS=		fetchmail.man

pys=			fetchmailconf.py
pym=			fetchmailconf.man

nodist_bin_SCRIPTS=	fetchmailconf
python_PYTHON=		$(pys)
dist_man1_MANS+=	$(pym)

CLEANFILES=		$(nodist_bin_SCRIPTS) \
			fetchmail-man.html \
			fetchmail.html \
			fetchmail.rst

# for gettext (used by fetchmail.c, NOT by GNU gettext)
localedir=		$(datadir)/locale
DEFS=			@DEFS@ -DLOCALEDIR=\"$(localedir)\"

noinst_LIBRARIES=	libfm.a
libfm_a_SOURCES=	xmalloc.c base64.c rfc822.c report.c rfc2047e.c \
			servport.c ntlm.h smbbyteorder.h smbdes.h smbmd4.h \
			smbencrypt.h smbdes.c smbencrypt.c smbmd4.c smbutil.c \
			smbtypes.h fm_getaddrinfo.c starttls.c rfc822valid.c \
			xmalloc.h sdump.h sdump.c x509_name_match.c \
			fm_strl.h md5c.c tls-aux.c tls-aux.h

if NTLM_ENABLE
libfm_a_SOURCES += ntlmsubr.c
endif
libfm_a_LIBADD=		$(EXTRAOBJ)
libfm_a_DEPENDENCIES=	$(EXTRAOBJ)
LDADD	=		libfm.a @LIBINTL@ $(LIBOBJS)
DEPENDENCIES=		libfm.a $(LIBOBJS)

check_PROGRAMS=

TESTS=			t.smoke t.validate-xhtml10 t.validate-xhtml t.x509_name_match t.realpath t.tls-aux \
			t.operation
LOG_COMPILER=		env LC_ALL=C TZ=UTC $(SHELL)

rcfile_l.h:	rcfile_l.l
	$(SHELL) $(YLWRAP) $(srcdir)/rcfile_l.l lex.yy.c $(srcdir)/rcfile_l.c rcfile_l.h $(srcdir)/rcfile_l.h -- $(LEXCOMPILE)

fetchmail_SOURCES=	fetchmail.h getopt.h \
		gettext.h i18n.h kerberos.h fm_md5.h mx.h netrc.h smtp.h \
		socket.h tunable.h \
		socket.c getpass.c \
		fetchmail.c env.c idle.c options.c daemon.c \
		driver.c transact.c sink.c smtp.c \
		idlist.c uid.c mxget.c md5ify.c cram.c gssapi.c \
		opie.c interface.c netrc.c \
		unmime.c conf.c checkalias.c uid_db.h uid_db.c\
		lock.h lock.c \
		rcfile_l.l rcfile_y.y \
		ucs/norm_charmap.c ucs/norm_charmap.h \
		fm_realpath.c
if POP2_ENABLE
fetchmail_SOURCES += pop2.c
endif
if POP3_ENABLE
fetchmail_SOURCES += pop3.c
endif
if IMAP_ENABLE
fetchmail_SOURCES += imap.c
endif
if ETRN_ENABLE
fetchmail_SOURCES += etrn.c
endif
if ODMR_ENABLE
fetchmail_SOURCES += odmr.c
endif
if KERBEROS_V4_ENABLE
fetchmail_SOURCES += kerberos.c
endif
if RPA_ENABLE
fetchmail_SOURCES += rpa.c
endif

tls_aux_SOURCES = 	tls-aux.c

check_PROGRAMS +=	rfc822 unmime netrc rfc2047e mxget rfc822valid \
			x509_name_match fm_realpath tls-aux

fm_realpath_CFLAGS=	-DTEST

rfc2047e_CFLAGS=	-DTEST

rfc822valid_CFLAGS=	-DTEST

rfc822_CFLAGS=		-DTEST

tls_aux_CFLAGS=		-DTEST

x509_name_match_CFLAGS=	-DTEST

unmime_SOURCES=	unmime.c
unmime_CFLAGS=	-DSTANDALONE -DHAVE_CONFIG_H -I$(builddir)

netrc_SOURCES=	netrc.c xmalloc.c report.c
netrc_CFLAGS=	-DSTANDALONE -DHAVE_CONFIG_H -I$(builddir)

mxget_SOURCES=	mxget.c
mxget_CFLAGS=	-DSTANDALONE -DHAVE_CONFIG_H -I$(builddir)

@SET_MAKE@

fetchmail.spec: Makefile.in specgen.sh
	$(srcdir)/specgen.sh $(VERSION) >fetchmail.spec

DISTDOCS=	FAQ FEATURES NOTES OLDNEWS fetchmail-man.html \
		design-notes.html esrs-design-notes.html todo.html \
		fetchmail-features.html README.SSL README.NTLM \
		README.packaging README.SSL-SERVER \
		fetchmail-FAQ.book fetchmail-FAQ.pdf fetchmail-FAQ.html \
		Mailbox-Names-UTF7.txt Mailbox-Names-UTF7.html \
		fetchmail-SA-2021-01.txt \
		fetchmail-SA-2012-02.txt \
		fetchmail-SA-2012-01.txt \
		fetchmail-SA-2011-01.txt \
		fetchmail-EN-2010-03.txt \
		fetchmail-SA-2010-02.txt \
		fetchmail-SA-2010-01.txt \
		fetchmail-SA-2009-01.txt \
		fetchmail-SA-2008-01.txt \
		fetchmail-SA-2007-02.txt \
		fetchmail-SA-2007-01.txt \
		fetchmail-SA-2006-03.txt \
		fetchmail-SA-2006-02.txt \
		fetchmail-SA-2006-01.txt \
		fetchmail-SA-2005-01.txt \
		fetchmail-SA-2005-02.txt \
		fetchmail-SA-2005-03.txt

doc_list=README README.NTLM README.SSL-SERVER README.SSL COPYING

# extra directories to ship
distdirs = rh-config contrib beos

EXTRA_DIST=	$(DISTDOCS) $(distdirs) \
		fetchmail.spec fetchmail.xpm \
		strlcpy.3 bighand.png \
		rcfile_l.h \
		m4/codeset.m4 \
		m4/gettext.m4 \
		m4/glibc2.m4 \
		m4/glibc21.m4 \
		m4/iconv.m4 \
		m4/intdiv0.m4 \
		m4/intl.m4 \
		m4/intldir.m4 \
		m4/intlmacosx.m4 \
		m4/intmax.m4 \
		m4/inttypes-pri.m4 \
		m4/inttypes_h.m4 \
		m4/lcmessage.m4 \
		m4/lib-ld.m4 \
		m4/lib-link.m4 \
		m4/lib-prefix.m4 \
		m4/lock.m4 \
		m4/longlong.m4 \
		m4/nls.m4 \
		m4/po.m4 \
		m4/printf-posix.m4 \
		m4/progtest.m4 \
		m4/size_max.m4 \
		m4/stdint_h.m4 \
		m4/uintmax_t.m4 \
		m4/visibility.m4 \
		m4/wchar_t.m4 \
		m4/wint_t.m4 \
		m4/xsize.m4 \
		m4-local/ac-archive-license.txt \
		m4-local/ac_ma_search_package.m4 \
		$(TESTS) t.rc t.operation.mock-pop3-server.py \
		dist-tools/html2txt \
		dist-tools/git-commit-po-updates.sh \
		delete-colgroup.sed \
		Doxyfile

fetchmailconf:
	( echo '#! /bin/sh' && echo 'exec @PYTHON@ @pythondir@/fetchmailconf.py "$$@"' ) >$@ && chmod +x $@

FAQ: fetchmail-FAQ.html $(srcdir)/dist-tools/html2txt
	AWK=$(AWK) $(SHELL) $(srcdir)/dist-tools/html2txt $(srcdir)/fetchmail-FAQ.html >$@

fetchmail-FAQ.pdf: fetchmail-FAQ.html fetchmail-FAQ.book bighand.png
	$(AWK) '/^[ \t]*<h1/ {sec++; } {if (sec < 2 || sec > 3) print $0;}' <$(srcdir)/fetchmail-FAQ.html >fetchmail-FAQ-print.html
	$(HTMLDOC) --logoimage $(srcdir)/bighand.png --batch $(srcdir)/fetchmail-FAQ.book
	rm -f fetchmail-FAQ-print.html

FEATURES: fetchmail-features.html $(srcdir)/dist-tools/html2txt
	AWK=$(AWK) $(SHELL) $(srcdir)/dist-tools/html2txt $(srcdir)/fetchmail-features.html >$@

NOTES: design-notes.html esrs-design-notes.html $(srcdir)/dist-tools/html2txt
	   echo "This file contains two articles reformatted from HTML." > $@ \
	&& echo "------------------------------------------------------" >> $@ \
	&& echo "" >> $@ \
	&& AWK=$(AWK) $(SHELL) $(srcdir)/dist-tools/html2txt $(srcdir)/design-notes.html >>$@ \
	&& AWK=$(AWK) $(SHELL) $(srcdir)/dist-tools/html2txt $(srcdir)/esrs-design-notes.html >>$@

TODO: todo.html $(srcdir)/dist-tools/html2txt
	AWK=$(AWK) $(SHELL) $(srcdir)/dist-tools/html2txt $(srcdir)/todo.html >$@

fetchmail-man.html: fetchmail.html delete-colgroup.sed
	sed -E -f $(srcdir)/delete-colgroup.sed $< >$@

dist-hook: fetch-translations
	cd $(distdir) && find $(distdirs) po -name .git -type d -prune -exec rm -rf '{}' ';'
	cd $(distdir) && find $(distdirs) po -name '*~' -exec rm -f '{}' ';'
	if ! $(top_srcdir)/dist-tools/git-commit-po-updates.sh -n ; then \
		printf '\n*** There are new translations. Please add them to the distribution. ***\n\n' ; exit 1 ; \
	fi
	test -r $(distdir)/rcfile_l.h && touch $(distdir)/rcfile_l.h # mark updated to avoid distcheck from breaking

# this target expects a .rsyncs file with lines of this format:
# host:directory/
# it will call rsync from its source directory to the destination for
# each of them, running them in parallel
rsync:  distdir $(srcdir)/.rsyncs
	@( cat $(srcdir)/.rsyncs | sed -e 's}^}rsync -aH --delete-after $(PACKAGE)-$(VERSION)/ }; s/\($$\)/ \&/;' ; echo "wait" ) | $(SHELL) -x

.PHONY: import-translations fetch-translations git-check
fetch-translations:
	rsync -Lrtvz translationproject.org::tp/latest/fetchmail/  $(top_srcdir)/po

import-translations: fetch-translations
	cd $(top_builddir)/po && $(MAKE) update-po
	@printf '\n=== You can use dist-tools/git-commit-po-updates.sh to check these updates in. ===\n\n'
	$(SHELL) $(top_srcdir)/dist-tools/git-commit-po-updates.sh -n

git-check:
	@set -x && if test -d $(PACKAGE)-$(VERSION)-export ; then chmod -R u+rwX $(PACKAGE)-$(VERSION)-export ; fi && \
	rm -rf $(PACKAGE)-$(VERSION)-export && \
	mkdir $(PACKAGE)-$(VERSION)-export && \
	( cd $(top_srcdir) && git archive --format=tar HEAD ) | tar -C $(PACKAGE)-$(VERSION)-export -xf - && \
	cd $(PACKAGE)-$(VERSION)-export && chmod -R u+rwX . && autoreconf -i -f -s -v \
	&& mkdir _build && cd _build && ../configure && $(MAKE) distcheck

SUFFIXES = .html .txt .rst
.txt.html:
	asciidoc --unsafe --attribute toc --attribute data-uri -o $@ $<
.man.rst:
	pandoc --from man --to rst --table-of-contents --standalone --output $@ $<
.rst.html:
	$(RST2HTML) --title "Fetchmail Manual" --strip-comments --rfc-references $< $@

# default to some non-default options when using "make distcheck"
AM_DISTCHECK_CONFIGURE_FLAGS=--with-ssl

# The following sets edit modes for GNU EMACS.
# Local Variables:
# compile-command:"configure"
# End:
install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/$$f.fetchmail || exit 1 ;\
	done
