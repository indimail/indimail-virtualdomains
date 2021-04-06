@SET_MAKE@
ACLOCAL_AMFLAGS=-I m4

htmllibdir=@htmllibdir@
htmldir=@htmldir@
imagedir=@imagedir@

version=@VERSION@

EXTRA_DIST=BUGS FAQ TRANSLATORS html lang images contrib

bindir=@cgibindir@
bin_PROGRAMS=@CGIBINTARGETS@
EXTRA_PROGRAMS=iwebadmin

man_MANS=iwebadmin.1

iwebadmin_DEPENDENCIES=$(COMMONDEPENDENCIES)

COMMONSOURCES=iwebadmin.c alias.c autorespond.c forward.c mailinglist.c \
	      user.c util.c printh.c auth.c template.c \
	      command.c show.c cgi.c limits.c dotqmail.c \
		  common.c iwebadmin.h iwebadminx.h

iwebadmin_SOURCES=$(COMMONSOURCES) 
iwebadmin_LDADD=$(LIB_INDIMAIL) $(LIB_QMAIL) $(LIB_CRYPT)
iwebadmin_LDFLAGS = $(AM_LDFLAGS)
iwebadmin_CFLAGS = $(AM_CFLAGS)

noinst_DATA = iwebadmin.spec iwebadmin.changes iwebadmin-rpmlintrc \
			  iwebadmin-permissions.easy iwebadmin-permissions.secure \
			  iwebadmin-permissions.paranoid iwebadmin-conf \
			  iwebadmin-hooks PKGBUILD

SYSCONFDIR = @sysconfdir@
sysconf_DATA = iwebadmin.perms iwebadmin-release

MOSTLYCLEANFILES = iwebadmin.perms

edit = sed \
	-e 's,@mysql_prefix\@,$(mysql_prefix),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@docdir\@,$(docdir),g' \
	-e 's,@htmldir\@,$(htmldir),g' \
	-e 's,@htmllibdir\@,$(htmllibdir),g' \
	-e 's,@sys_libdir\@,$(sys_libdir),g' \
	-e 's,@cgibindir\@,$(cgibindir),g' \
	-e 's,@imagedir\@,$(imagedir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's,@prefix\@,$(prefix),g'

iwebadmin.o: config.h iwebadmin.h 
alias.o: config.h iwebadmin.h iwebadmin.h 
autorespond.o: config.h iwebadmin.h iwebadmin.h 
forward.o: config.h iwebadmin.h iwebadminx.h 
mailinglist.o: config.h iwebadmin.h iwebadminx.h 
user.o: config.h iwebadmin.h iwebadminx.h 
util.o: config.h iwebadmin.h iwebadminx.h 
printh.o: printh.h
auth.o: config.h iwebadmin.h iwebadminx.h 
template.o: config.h iwebadmin.h iwebadminx.h 
command.o: iwebadmin.h iwebadminx.h 
show.o: config.h iwebadmin.h iwebadminx.h 
cgi.o: config.h iwebadmin.h iwebadminx.h 
limits.o: config.h iwebadmin.h iwebadminx.h 
dotqmail.o: config.h iwebadmin.h iwebadminx.h 

iwebadmin.spec: iwebadmin.spec.in conf-version conf-release doc/ChangeLog conf-email
	(cat $@.in;./catChangeLog --spec doc/ChangeLog) | $(edit)> $@
iwebadmin.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release conf-email \
iwebadmin.changes
	cat $@.in | $(edit) > $@
iwebadmin-rpmlintrc: iwebadmin-rpmlintrc.in
	$(edit) iwebadmin-rpmlintrc.in > $@
debian/Makefile: debian/Makefile.in conf-version conf-release
	$(edit) $@.in > $@
iwebadmin.perms: perm_list.in
	$(edit) perm_list.in > $@
iwebadmin-permissions.easy: iwebadmin-permissions.easy.in
	$(edit) $@.in > $@
iwebadmin-permissions.secure: iwebadmin-permissions.secure.in
	$(edit) $@.in > $@
iwebadmin-permissions.paranoid: iwebadmin-permissions.paranoid.in
	$(edit) $@.in > $@

# Maintainer stuff

dist-hook:

# use the [^C]* pattern to skip the CVS directory
install-data-local:
	$(mkinstalldirs)                $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) iwebadmin-conf  $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) iwebadmin-hooks $(DESTDIR)$(sysconfdir)
	$(mkinstalldirs)                $(DESTDIR)@htmllibdir@
	rm -rf                          $(DESTDIR)@htmllibdir@/html
	$(mkinstalldirs)                $(DESTDIR)@htmllibdir@/html 
	cp $(srcdir)/html/*.html        $(DESTDIR)@htmllibdir@/html
	cp $(srcdir)/html/colortable    $(DESTDIR)@htmllibdir@/html
	chmod 755                       $(DESTDIR)@htmllibdir@/html
	chmod 644                       $(DESTDIR)@htmllibdir@/html/*
	rm -rf                          $(DESTDIR)@htmllibdir@/lang
	$(mkinstalldirs)                $(DESTDIR)@htmllibdir@/lang 
	cp $(srcdir)/lang/??            $(DESTDIR)@htmllibdir@/lang
	cp $(srcdir)/lang/??-??         $(DESTDIR)@htmllibdir@/lang
	chmod 755                       $(DESTDIR)@htmllibdir@/lang
	chmod 644                       $(DESTDIR)@htmllibdir@/lang/*
	$(mkinstalldirs)                $(DESTDIR)@imagedir@
	cp $(srcdir)/images/*.png       $(DESTDIR)@imagedir@

	if test `@idcommand@` = "0" ; then \
		echo "chown root $(DESTDIR)$(bindir)/iwebadmin"; \
		chown root $(DESTDIR)$(bindir)/iwebadmin; \
		echo "chmod 4555 $(DESTDIR)$(bindir)/iwebadmin"; \
		chmod 4555 $(DESTDIR)$(bindir)/iwebadmin; \
	fi
install-data-hook:
	$(mkinstalldirs)                $(DESTDIR)@docdir@
	$(INSTALL_DATA) README.md       $(DESTDIR)@docdir@/README.md || exit 1
	$(INSTALL_DATA) README-hooks.md $(DESTDIR)@docdir@/README-hooks.md || exit 1
	$(INSTALL_DATA) COPYING         $(DESTDIR)@docdir@/COPYING || exit 1
	$(INSTALL_DATA) AUTHORS         $(DESTDIR)@docdir@/AUTHORS || exit 1

AUTOMAKE_OPTIONS = foreign no-dependencies
