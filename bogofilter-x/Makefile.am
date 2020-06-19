# automake options
AUTOMAKE_OPTIONS = 

COFLAGS=-z+05:30
# search path for m4 macros
ACLOCAL_AMFLAGS = -I m4

release=@release@
spamfilter_version=@spamfilter_version@
SUBDIRS = trio src . doc
SYSCONFDIR = @sysconfdir@

# what to build
sysconf_DATA=	bogofilter.cf.example perm_list.bogofilter
MOSTLYCLEANFILES=perm_list.bogofilter

# ChangeLog, cvs2cl lives at http://www.red-bean.com/cvs2cl
.PHONY:		$(srcdir)/changelog.txt
$(srcdir)/changelog.txt:
		test -f $@ || : >$@ || { rm -f $@ ; exit 1; }
		( cd $(srcdir) && cvs2cl.pl --fsf --accum --utc --file changelog.txt )

# what to distribute
EXTRA_DIST=	autogen.sh gsl config.rpath \
		bogofilter.cf.example \
		bogofilter.spec.in \
		bogofilter.spec \
		Doxyfile gpl-3.0.txt gnugetopt \
		GETTING.STARTED \
		NEWS.0 \
		OBITUARY \
		README.git \
		RELEASE.NOTES

.PHONY:	check rpm git-check

doc_list=AUTHORS HOWTO COPYING

#
# RPM building - there are some cheats here
#
# NOTE this is unsupported and left to packagers.
# The information below is historic.
#
# BF_ZAP_LIBDB is a magic variable we check from configure.ac,
# which zaps LIBDB and allows us to override the library choice
# through LIBS=...
#
# CPPFLAGS=-D__NO_CTYPE tells glibc to use the older glibc 2.2.X
# ctype implementation (functions). Else glibc would use a newer
# 2.3.X ctype implementation, and as this is the only module that
# requires 2.3.X in our code, we just use the 2.2.X version instead.
# This works up to and including 2.3.5, the newest glibc version
# available at the time this code was written.
#
# RPM and GPG-AGENT
# =================
# Using the GnuPG agent requires some fiddling, as RPM has no native
# support.
#
# 1. add these lines to your ~/.rpmmacros:
#
# %__gpg_check_password_cmd   %{__gpg} gpg --batch --no-verbose \
#    --use-agent -u "%{_gpg_name}" -so -
# %__gpg_sign_cmd             %{__gpg} gpg --batch --no-verbose \
#    --no-armor --use-agent --no-secmem-warning -u "%{_gpg_name}" \
#    -sbo %{__signature_filename} %{__plaintext_filename}
#
# 2. launch the GnuPG agent, unless already running:
#    eval $(gpg-agent --sh --daemon)
#
# 3. run "setsid make rpm </dev/null >/dev/null"
#
rpm:
	@echo
	@echo "=================================="
	@echo "'make rpm' is no longer supported."
	@echo "=================================="
	@echo
	@exit 1
	@echo
	@echo "Note: if 'make rpm' fails, check the top-level Makefile.am for documentation."
	@echo
	@echo "Note: see Makefile.am comments for information on how to use the GnuPG agent."
	@echo
	@echo "Now building the shared database library RPMs - this may fail"
	@echo "if system-wide library installs of BerkeleyDB 5.3 or SQLite 3"
	@echo "are missing from your system."
	@echo
	@sleep 5
	-                CPPFLAGS="-D__NO_CTYPE -I$(DBPFX)" LIBS="-ldb-5.3 -pthread" \
			$(RPMBUILD)              $(DEF_DB42)   $(SIGN) -tb $(distdir).tar.gz
	-                CPPFLAGS="-D__NO_CTYPE -I$(SQPFX)"               \
			$(RPMBUILD)              $(DEF_SQLITE) $(SIGN) -tb $(distdir).tar.gz

git-check:
	if test -d $(PACKAGE)-$(VERSION)-export ; then chmod -R u+rwX $(PACKAGE)-$(VERSION)-export ; fi
	rm -rf $(PACKAGE)-$(VERSION)-export
	mkdir $(PACKAGE)-$(VERSION)-export
	( cd $(top_srcdir) && git archive --format=tar HEAD ) | tar -C $(PACKAGE)-$(VERSION)-export -xf -
	cd $(PACKAGE)-$(VERSION)-export && chmod -R u+rwX . && autoreconf -i -f -s -v \
	&& mkdir _build && cd _build && ../configure && $(MAKE) distcheck

# this target expects a .rsyncs file with lines of this format:
# host:directory/
# it will call rsync from its source directory to the destination for
# each of them, running them in parallel
.PHONY: rsync rsynconly
rsync:	distdir rsynconly
rsynconly: .rsyncs
	@( cat $(srcdir)/.rsyncs | sed -e 's}^}rsync -av -e ssh --delete $(PACKAGE)-$(VERSION)/ }; s/\($$\)/ \&/;' ; echo "wait" ) | $(SHELL) -x

install-data-hook:
	chown root:indimail $(DESTDIR)$(bindir)/bogofilter || true
	chmod 6511 $(DESTDIR)$(bindir)/bogofilter || true
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/$$f.bogofilter || exit 1 ;\
	done

dist-hook:
	cd $(distdir) && find doc gnugetopt gsl '(' -name .svn -o -name .git -o -name CVS ')' -type d -prune -exec rm -rf '{}' +
	cd $(distdir) && find doc gnugetopt gsl -name '.#*' -type f -exec rm '{}' ';'
	rm -f $(distdir)/doc/.cvsignore $(distdir)/ChangeLog

# ---------------------------------------------------------------------
# This section deals with the contrib/ directory
#
noinst_PROGRAMS=bogogrep
bogogrep_SOURCES=contrib/bogogrep.c
noinst_DATA = indimail-spamfilter.spec indimail-spamfilter-rpmlintrc \
			  indimail-spamfilter-permissions.easy indimail-spamfilter-permissions.secure \
			  indimail-spamfilter-permissions.paranoid

EXTRA_DIST+= \
	contrib/README.contrib \
	contrib/README.randomtrain \
	contrib/bogo.R \
	contrib/bfproxy.pl \
	contrib/bogofilter-milter.pl \
	contrib/bogofilter-qfe.sh \
	contrib/bogominitrain.pl \
	contrib/dot-qmail-bogofilter-default \
	contrib/mailfilter.example \
	contrib/mime.get.rfc822.pl \
	contrib/parmtest.sh \
	contrib/printmaildir.pl \
	contrib/procmailrc.example \
	contrib/randomtrain.sh \
	contrib/scramble.sh \
	contrib/spamitarium.pl \
	contrib/stripsearch.pl \
	contrib/trainbogo.sh \
	contrib/vm-bogofilter.el

edit = sed \
	-e 's}@servicedir\@}$(servicedir)}g' \
	-e 's}@sysconfdir\@}$(sysconfdir)}g' \
	-e 's}@mandir\@}$(mandir)}g' \
	-e 's}@libexecdir\@}$(libexecdir)}g' \
	-e 's}@libdir\@}$(libdir)}g' \
	-e 's}@logdir\@}$(logdir)}g' \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e 's}@spamfilter_version\@}$(spamfilter_version)}g' \
	-e 's|@release\@|$(release)|g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@DESTDIR\@}$(DESTDIR)}g' \
	-e 's}@HOST\@}$(host)}g' \
	-e 's}@DATE\@}$(DATE)}g' \
	-e 's}@prefix\@}$(prefix)}g'

indimail-spamfilter.spec: indimail-spamfilter.spec.in catChangeLog doc/ChangeLog conf-version conf-release
	(cat $@.in;./catChangeLog) | $(edit) > $@
debian/Makefile: debian/Makefile.in configure.ac
	$(edit) $@.in > $@;
perm_list.bogofilter: perm_list.in
	$(edit) perm_list.in > $@;
indimail-spamfilter-rpmlintrc: indimail-spamfilter-rpmlintrc.in
	$(edit) indimail-spamfilter-rpmlintrc.in > $@
indimail-spamfilter-permissions.easy: indimail-spamfilter-permissions.easy.in
	$(edit) $@.in > $@
indimail-spamfilter-permissions.secure: indimail-spamfilter-permissions.secure.in
	$(edit) $@.in > $@
indimail-spamfilter-permissions.paranoid: indimail-spamfilter-permissions.paranoid.in
	$(edit) $@.in > $@
# ---------------------------------------------------------------------
