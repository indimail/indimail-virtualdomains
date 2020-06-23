@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
version=@VERSION@

noinst_DATA = ircube.spec

ircubelibexecdir=@libexecdir@
ircubelibexec_SCRIPTS = bogo-learn roundcube_config

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(version),g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's,@prefix\@,$(prefix),g'

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@plugindir@ || exit 1
	cp -R plugins $(DESTDIR)@plugindir@ || exit 1
	$(mkinstalldirs) $(DESTDIR)@prefix@/share/indimail/doc || exit 1
	cp README.md $(DESTDIR)@prefix@/share/indimail/doc/README-ircube.md || exit 1
	cp COPYING $(DESTDIR)@prefix@/share/indimail/doc/COPYING.ircube || exit 1

ircube.spec: ircube.spec.in catChangeLog conf-version conf-release ChangeLog
	($(edit) $@.in;./catChangeLog) | $(edit) > $@
