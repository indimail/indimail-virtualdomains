@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
version=@VERSION@

noinst_DATA = ircube.spec ircube.changes PKGBUILD

ircubelibexecdir=@libexecdir@
ircubelibexec_SCRIPTS = bogo-learn roundcube_config

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(version),g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's,@prefix\@,$(prefix),g'

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@plugindir@ || exit 1
	cp -R plugins $(DESTDIR)@plugindir@ || exit 1
	$(mkinstalldirs) $(DESTDIR)@prefix@/share/indimail/doc || exit 1
	cp README.md $(DESTDIR)@prefix@/share/indimail/doc/README-ircube.md || exit 1
	cp COPYING $(DESTDIR)@prefix@/share/indimail/doc/COPYING.ircube || exit 1

ircube.spec: ircube.spec.in catChangeLog conf-version conf-release \
doc/ChangeLog conf-email
	(cat $@.in;./catChangeLog) | $(edit) > $@
ircube.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release \
conf-email ircube.changes
	cat $@.in | $(edit) > $@
