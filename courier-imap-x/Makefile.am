#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.

@SET_MAKE@

SUBDIRS=libs/numlib libs/bdbobj libs/gdbmobj libs/soxwrap \
	libs/unicode libs/rfc822 libs/rfc2045 \
	libs/md5 libs/sha1 libs/random128 libs/rfc1035 libs/liblock \
	libs/maildir libs/makedat libs/libhmac libs/authlib \
	libs/waitlib libs/tcpd libs/rootcerts libs/imap libs/maildrop

DISTCLEANFILES=stamp-h1 config.h config.log packaging/systemd/courier-imap.sysvinit packaging/systemd/courier-imap.service \
	packaging/suse/courier-imap.init pop3d.rc dbobj.h imapd-ssl.rc pop3d-ssl.rc imapd.rc dbobj.config $(MAILDROPHTML)

#CLEANFILES=$(databin_SCRIPTS) $(man_MANS) $(sysconf_DATA) $(sbin_SCRIPTS)
CLEANFILES=$(sysconf_DATA)

doc_list=libs/maildir/README.sharedfolders.md libs/maildir/README.maildirquota.txt \
	libs/maildir/README.imapkeywords.txt libs/unicode/README.unicode \
	libs/imap/README.imap libs/tcpd/README.couriertls \
	README.courier-imap AUTHORS COPYING COPYING.GPL

EXTRA_DIST=README.courier-imap README.cygwin\
	COPYING.GPL NEWS.html pgpkeys.txt \
	INSTALL.html maildroptips.txt manpage.css

if HAVE_SGML
manpage.css: libs/docbook/manpage.css
	cp -f $(srcdir)/libs/docbook/manpage.css manpage.css
endif

htmldir=$(docdir)/html
MAILDROPHTML=lockmail.html maildiracl.html maildirkw.html \
	maildirmake.html \
	maildir.html \
	maildropfilter.html maildrop.html \
	makemime.html reformime.html \
	mailbot.html maildropex.html \
	maildropgdbm.html reformail.html

html_DATA=$(MAILDROPHTML) manpage.css

sysconf_DATA=quotawarnmsg.example
libexec_SCRIPTS=imapd.rc imapd-ssl.rc pop3d.rc pop3d-ssl.rc

HTML2TXT=@links@ -dump -no-numbering -no-references

NEWS: NEWS.html
	$(HTML2TXT) NEWS.html >NEWS
distclean-local:
	-rm -rf .deps libs/*/.deps autom4te.cache libs/*/autom4te.cache

quotawarnmsg.example: libs/maildir/quotawarnmsg
	cp -f $(srcdir)/libs/maildir/quotawarnmsg quotawarnmsg.example
uninstall-hook:
	for f in `cat libs/authlib/modulelist ; echo $$authmodulelist ` ; do \
		/bin/rm -f $(DESTDIR)$(libexecdir)/imapmodules/$$f; \
	done
	/bin/rmdir $(DESTDIR)$(libexecdir)/imapmodules

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libexecdir)/imapmodules
	for f in `cat libs/authlib/installlist` dummy ; do \
		test "$$f" = "dummy" && continue ; \
		echo $$f; \
		$(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) libs/authlib/$$f \
			$(DESTDIR)$(libexecdir)/imapmodules/$$f ; done
	for f in `cat libs/authlib/modulelist ; echo $$authmodulelist ` ; do \
		echo $$f; \
		test "$$f" = "authpam" || test "$$f" = "authshadow" || continue ; \
		chmod 4555 $(DESTDIR)$(libexecdir)/imapmodules/$$f || exit 1; \
	done
	$(mkinstalldirs) $(DESTDIR)$(databindir)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	test ! -x libs/tcpd/couriertls || $(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) \
		libs/tcpd/couriertls $(DESTDIR)$(bindir)/couriertls

install-exec-hook:
	chmod 444 $(DESTDIR)$(sysconfdir)/quotawarnmsg.example
	mkdir -p $(DESTDIR)$(sysconfdir)/shared
	chmod 755 $(DESTDIR)$(sysconfdir)/shared
	mkdir -p $(DESTDIR)$(sysconfdir)/shared.tmp
	chmod 755 $(DESTDIR)$(sysconfdir)/shared.tmp
	mkdir -p $(DESTDIR)$(sysconfdir)/imapaccess
	chmod 755 $(DESTDIR)$(sysconfdir)/imapaccess

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	$(mkinstalldirs) $(DESTDIR)$(mandir)
	$(INSTALL_DATA) libs/authlib/modulelist $(DESTDIR)$(sysconfdir)/authmodulelist
	@echo "Do not forget to run make install-configure"
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc/courier-imap || exit 1
	for f in $(doc_list); do \
		j=`basename $$f`; \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/courier-imap/$$j || exit 1 ;\
	done

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/couriertls

#
# For maintainer's use only
#

DATE := $(shell date -R)

dist-hook: courier-imap.spec
	test -f $(srcdir)/rpm.release && cp $(srcdir)/rpm.release $(distdir) \
		; exit 0
	rm -f $(distdir)/INSTALL
	$(HTML2TXT) INSTALL.html >$(distdir)/INSTALL
	cp $(srcdir)/courier-imap.spec $(distdir)

if HAVE_HTML

README.courier-imap: html/main.html
	$(HTML2TXT) $(srcdir)/html/index.html >README.courier-imap
endif

distrelease:
	umask 022; $(MAKE) distcheck
	$(MAKE) rpm
	$(MAKE)
	$(MAKE)
	$(MAKE) release

release: dist

rpm: build-rpm

build-rpm:
	$(MAKE) courier-imap.spec
	$(MAKE) bump.rpm.release
	$(MAKE) dist
	rm -rf rpm/BUILD/*
	rm -f rpm/RPMS/*/*
	rm -f rpm/SOURCES/*
	rm -f rpm/SPECS/*
	rm -f rpm/SRPMS/*
	rpmbuild -ta --clean \
		--define "_topdir `pwd`/rpm" \
		--define '_rpmdir %{_topdir}/RPMS' \
		--define '_srcrpmdir %{_topdir}/SRPMS' \
		--define '_sourcedir %{_topdir}/SOURCES' \
		--define '_specdir %{_topdir}/SPECS' \
		--define '_builddir %{_topdir}/BUILD' \
		--define '_build_name_fmt %%{ARCH}/%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
		--define '_tmppath %{_var}/tmp' \
		--define '__spec_prep_pre %{___build_pre}' \
		@PACKAGE@-@VERSION@.tar.bz2
	$$HOME/bin/rpmrepos.sh

bump.rpm.release: @PACKAGE_NAME@.spec
	VERSION="x"; \
	test -f $(srcdir)/rpm.release && . $(srcdir)/rpm.release; \
	NEXT_RELEASE=0; \
	test "$$VERSION" = "@PACKAGE_VERSION@" && NEXT_RELEASE="$$RELEASE"; \
	RELEASE=`expr $$NEXT_RELEASE + 1`; \
	echo "VERSION=@PACKAGE_VERSION@" >$(srcdir)/rpm.release; \
	echo "RELEASE=$$RELEASE" >>$(srcdir)/rpm.release; \
	sed 's/^Release: .*/Release: '$$RELEASE'%{courier_release}/' \
		courier-imap.spec >courier-imap.spec.new ; \
	mv courier-imap.spec.new courier-imap.spec

all-recursive: all-make-check

all-make-check:
	$(MAKE) all-gmake-check FOO=BAR

all-gmake-check:
	@test "$(FOO)" = "BAR" && exit 0 ; echo "" ; \
		echo "ERROR: Something's wrong.  If you're using xBSD, you must use gmake" ; \
		echo "instead of make, as you were told by the INSTALL file (you did read the" ; \
		echo "INSTALL file, did you?)" ; echo "" ; exit 1

check-am:
	cd libs/imap; $(MAKE) check-version TOPLEVEL_VERSION="`echo $(VERSION) | cut -d. -f1-3`"

lockmail.html: libs/liblock/lockmail.html
	cp -f $(srcdir)/libs/liblock/lockmail.html lockmail.html
maildir.html: libs/maildir/maildir.html
	cp -f $(srcdir)/libs/maildir/maildir.html maildir.html
maildiracl.html: libs/maildir/maildiracl.html
	cp -f $(srcdir)/libs/maildir/maildiracl.html maildiracl.html
maildirkw.html: libs/maildir/maildirkw.html
	cp -f $(srcdir)/libs/maildir/maildirkw.html maildirkw.html
maildirmake.html: libs/maildir/maildirmake.html
	cp -f $(srcdir)/libs/maildir/maildirmake.html maildirmake.html
maildrop.html: libs/maildrop/maildrop.html
	cp -f $(srcdir)/libs/maildrop/maildrop.html maildrop.html
maildropfilter.html: libs/maildrop/maildropfilter.html
	cp -f $(srcdir)/libs/maildrop/maildropfilter.html maildropfilter.html
makemime.html: libs/rfc2045/makemime.html
	cp -f $(srcdir)/libs/rfc2045/makemime.html makemime.html
reformime.html: libs/rfc2045/reformime.html
	cp -f $(srcdir)/libs/rfc2045/reformime.html reformime.html
mailbot.html: libs/maildrop/mailbot.html
	cp -f $(srcdir)/libs/maildrop/mailbot.html mailbot.html
maildropex.html: libs/maildrop/maildropex.html
	cp -f $(srcdir)/libs/maildrop/maildropex.html maildropex.html
maildropgdbm.html: libs/maildrop/maildropgdbm.html
	cp -f $(srcdir)/libs/maildrop/maildropgdbm.html maildropgdbm.html
reformail.html: libs/maildrop/reformail.html
	cp -f $(srcdir)/libs/maildrop/reformail.html reformail.html

# Copyright 2000 Double Precision, Inc.  See COPYING for
# distribution information.

# Need to add sysconftoolize to package:

EXTRA_DIST += sysconftool

# NOTE: when using a complex directory hierarchy it is only necessary to
# include sysconftool in the top level directory, and it is not necessary
# to add sysconftool to EXTRA_DIST in subdirectories.

SYSCONFFILES=$(sysconf_DATA)

# SYSCONFFILES should be all of your .dist files, this is a default.
# You can also change install-configure-local target below, instead.

# If -- in a complex directory hierarchy -- a certain subdirectory does not
# need install-configure, leave it defined as an empty rule to prevent
# sysconftoolize from appending another form template to this Makefile.am

install-configure:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		( case "$$subdir" in .) continue;; \
			*) target=install-configure;; esac ; \
		cd $$subdir ; $(MAKE) install-configure DESTDIR=$(DESTDIR) 2>/dev/null) \
			; done ; exit 0
	$(MAKE) install-configure-local DESTDIR=$(DESTDIR)

# Modify the following target as appropriate:

install-configure-local:
	@list="$(SYSCONFFILES)"; \
		for file in $$list; do \
		file="$(DESTDIR)$(sysconfdir)/$$file" ; \
		case "$$file" in *.dist) \
			@SYSCONFTOOL@ $$file;; esac ; done
upload-beta:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2; test -f @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig || exit 1
	~/src/sftools/release.pl mrsam courier imap-devel `echo @PACKAGE_VERSION@|sed 's/.*\(........\)/\1/'` @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2 @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig

upload-dist:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2; test -f @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig || exit 1
	set -e; v="@PACKAGE_NAME@/@PACKAGE_VERSION@/`date +%Y%m%d%H%M%S`"; git tag -s $$v -m 'Tag version @PACKAGE_VERSION@'; cd libs; git tag -s $$v -m 'Tag version @PACKAGE_VERSION@'
	socksify ~/src/sftools/release.pl mrsam courier imap @PACKAGE_VERSION@ @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2 @PACKAGE_NAME@-@PACKAGE_VERSION@.tar.bz2.sig

dollar=$$
id=$(dollar)Id

WWW=$$HOME/www/hostrocket/courier-mta.org

htmldoc: INSTALL.html libs/imap/README.html $(MAILDROPHTML) \
	libs/maildrop/maildroptips.html libs/maildrop/mailbot.html libs/maildrop/maildropex.html \
	libs/maildrop/maildropgdbm.html libs/maildrop/reformail.html \
	libs/maildir/README.maildirquota.html libs/maildir/README.sharedfolders.html \
	libs/maildir/README.imapkeywords.html libs/imap/BUGS.html libs/imap/README.proxy.html libs/imap/README.html
	for f in $^; do xsltproc --nonet \
		$(srcdir)/imap-format.xsl $$f | perl -ne 's~\$(id):.*\$$~\$(id):'"$$h"'\$$~; \
		print' >$(WWW)/imap/`basename $$f`; done
	rm -rf doc
	mkdir doc
	xsltproc --nonet -o doc/ $(srcdir)/imap.xsl $(srcdir)/imap.xml
	for f in doc/*.html; do \
		echo $$f; \
		tidy -i -asxml --doctype transitional \
			--input-encoding utf8 --output-encoding ascii \
			-q --tidy-mark no $$f >$$f.tmp; \
		xsltproc --nonet $(WWW)/xsl/postproc.xsl \
			$$f.tmp > $$f.tmp2; \
		xsltproc --nonet $(srcdir)/imap-format.xsl \
			 $$f.tmp2 >$$f; \
		rm -f $$f.tmp*; done
	cp -f doc/*.html $(WWW)/imap
	cp -f courier-imap.png bg.png $(WWW)/imap
	cp manpage.css html
	rm -rf doc

AUTOMAKE_OPTIONS=dist-bzip2
