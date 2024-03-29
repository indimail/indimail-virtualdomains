ACLOCAL_AMFLAGS = -I m4
COFLAGS=-z+05:30
RM	= /bin/rm -f
DEVNULL = /dev/null
DIST_COMMON=COPYING AUTHORS
BUILT_SOURCES = procmail.spec procmail.changes procmail-rpmlintrc \
	procmail-permissions.easy procmail-permissions.secure \
	procmail-permissions.paranoid PKGBUILD

MOSTLYCLEANFILES=man/*.1 man/*.5 man/man.sed.* man/man.sed \
	procmail.perms formail.1 lockfile.1 procmail.1 \
	procmailex.5 procmailrc.5 procmailsc.5 debian/Makefile

bin_PROGRAMS=procmail lockfile formail
bin_SCRIPTS=mailstat

SYSCONFDIR = @sysconfdir@
sysconf_DATA = procmail.perms procmail-release

procmail_SOURCES=procmail.c cstdio.c common.c exopen.c goodies.c \
	locking.c mailfold.c foldinfo.c misc.c pipes.c regexp.c robust.c \
	sublib.c acommon.c mcommon.c lastdirsep.c lmtp.c memblk.c \
	variables.c from.c comsat.c authenticate.c
procmail_LDADD=$(LIBS)
procmail_DEPENDENCIES=cstdio.$(OBJEXT) common.$(OBJEXT) exopen.$(OBJEXT) goodies.$(OBJEXT) \
	locking.$(OBJEXT) mailfold.$(OBJEXT) foldinfo.$(OBJEXT) misc.$(OBJEXT) pipes.$(OBJEXT) \
	regexp.$(OBJEXT) robust.$(OBJEXT) sublib.$(OBJEXT) acommon.$(OBJEXT) mcommon.$(OBJEXT) \
	lastdirsep.$(OBJEXT) authenticate.$(OBJEXT) lmtp.$(OBJEXT) memblk.$(OBJEXT) \
	variables.$(OBJEXT) from.$(OBJEXT) comsat.$(OBJEXT)
lockfile_SOURCES=lockfile.c exopen.c sublib.c acommon.c mcommon.c lastdirsep.o authenticate.c
lockfile_DEPENDENCIES=exopen.$(OBJEXT) sublib.$(OBJEXT) acommon.$(OBJEXT) mcommon.$(OBJEXT) \
	lastdirsep.$(OBJEXT) authenticate.$(OBJEXT)
lockfile_LDADD=$(LIBS)
formail_SOURCES=common.c fields.c formisc.c sublib.c ecommon.c acommon.c
formail_DEPENDENCIES=common.$(OBJEXT) fields.$(OBJEXT) formisc.$(OBJEXT) sublib.$(OBJEXT) \
	ecommon.$(OBJEXT) acommon.$(OBJEXT)

authenticate.$(OBJEXT): authenticate.c
	$(COMPILE) -DPROCMAIL -DHAVE_CONFIG_H -c authenticate.c

procmail$(EXEEXT): procmail.$(OBJEXT) $(procmail_DEPENDENCIES)
	$(COMPILE) $(CFLAGS) $@.$(OBJEXT) $(procmail_DEPENDENCIES) -o $@ $(LDFLAGS) $(procmail_LDADD)

lockfile$(EXEEXT): lockfile.$(OBJEXT) $(lockfile_DEPENDENCIES)
	$(COMPILE) $(CFLAGS) $@.$(OBJEXT) $(lockfile_DEPENDENCIES) -o $@ $(LDFLAGS) $(lockfile_LDADD)

formail$(EXEEXT): formail.$(OBJEXT) $(formail_DEPENDENCIES)
	$(COMPILE) $(CFLAGS) $@.$(OBJEXT) $(formail_DEPENDENCIES) -o $@ $(LDFLAGS)

man_MANS=procmail.1 formail.1 lockfile.1 mailstat.1 procmailrc.5 procmailsc.5 procmailex.5

procmail.1: man/procmail.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) procmail.man $@ "$(RM)" $(DEVNULL)
	@cp man/procmail.1 .
formail.1: man/formail.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) formail.man $@ "$(RM)" $(DEVNULL)
	@cp man/formail.1 .
lockfile.1: man/lockfile.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) lockfile.man $@ "$(RM)" $(DEVNULL)
	@cp man/lockfile.1 .
procmailrc.5: man/procmailrc.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) procmailrc.man $@ "$(RM)" $(DEVNULL)
	@cp man/procmailrc.5 .
procmailsc.5: man/procmailsc.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) procmailsc.man $@ "$(RM)" $(DEVNULL)
	@cp man/procmailsc.5 .
procmailex.5: man/procmailex.man man/man.sed
	cd man;$(SHELL) mansed $(SHELL) procmailex.man $@ "$(RM)" $(DEVNULL)
	@cp man/procmailex.5 .

man/man.sed: manconf.c config.h includes.h procmail.h
	$(CC) -O -DHAVE_CONFIG_H "-DBINDIR=\"$(prefix)/bin\"" manconf.c -o _autotst
	@./_autotst $@
	@echo Housekeeping file >$@
	@/bin/rm -f _autotst

edit = sed \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@DESTDIR\@,$(DESTDIR),g' \
	-e 's,@HOST\@,$(host),g' \
	-e 's,@DATE\@,$(DATE),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@prefix\@,$(prefix),g'

procmail-permissions.easy: procmail-permissions.easy.in
	$(edit) procmail-permissions.easy.in > $@
procmail-permissions.secure: procmail-permissions.secure.in
	$(edit) procmail-permissions.secure.in > $@
procmail-permissions.paranoid: procmail-permissions.paranoid.in
	$(edit) procmail-permissions.paranoid.in > $@
procmail-rpmlintrc: procmail-rpmlintrc.in conf-version conf-release
	$(edit) procmail-rpmlintrc.in > $@
procmail.spec: procmail.spec.in conf-version conf-release \
doc/ChangeLog conf-email
	(cat $@.in;./catChangeLog) | $(edit) > $@
procmail.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release conf-email \
procmail.changes
	cat $@.in | $(edit) > $@
debian/Makefile: debian/Makefile.in conf-version conf-release
	$(edit) $@.in > $@
procmail.perms: perm_list.in
	$(edit) perm_list.in > $@

install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)@docdir@
	$(INSTALL_DATA) procmail.perms $(DESTDIR)$(sysconfdir)/procmail.perms  || exit 1
	if test `@idcommand@` = "0" ; then \
		chown root:indimail $(DESTDIR)$(bindir)/procmail || true; \
		chmod 4511 $(DESTDIR)$(bindir)/procmail || true; \
	fi
	$(INSTALL_DATA) AUTHORS $(DESTDIR)@docdir@/AUTHORS || exit 1
	$(INSTALL_DATA) COPYING $(DESTDIR)@docdir@/COPYING || exit 1
	$(INSTALL_DATA) INSTALL.procmail $(DESTDIR)@docdir@/INSTALL.procmail || exit 1
