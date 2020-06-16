@SET_MAKE@
ACLOCAL_AMFLAGS=-I m4
DEFS=@DEFS@ -fno-strict-aliasing -I. @auth_incs@

htmllibdir=@htmllibdir@
htmldir=@htmldir@
imagedir=@imagedir@

version=$(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null|| echo "1.1")

EXTRA_DIST=BUGS FAQ TRANSLATORS html lang images contrib

#
# Install iwebadmin setuid to @indimailuser@
#
INSTALL_PROGRAM = ${INSTALL} -c
INSTALL_STRIP_PROGRAM=${INSTALL_PROGRAM} -s

bindir=@cgibindir@
bin_PROGRAMS=@CGIBINTARGETS@
EXTRA_PROGRAMS=iwebadmin

SYSCONFDIR = @sysconfdir@
sysconf_DATA = perm_list.iwebadmin

iwebadmin_DEPENDENCIES=$(COMMONDEPENDENCIES)

COMMONSOURCES=iwebadmin.c alias.c autorespond.c forward.c mailinglist.c \
	      user.c util.c printh.c auth.c template.c \
	      command.c show.c cgi.c limits.c dotqmail.c \
		  common.c \
	      iwebadmin.h iwebadminx.h

iwebadmin_SOURCES=$(COMMONSOURCES) 
iwebadmin_LDADD=@auth_libs@
iwebadmin_CFLAGS = @auth_incs@ $(AM_CFLAGS)

noinst_DATA = iwebadmin.spec iwebadmin-rpmlintrc \
			  iwebadmin-permissions.easy iwebadmin-permissions.secure \
			  iwebadmin-permissions.paranoid

MOSTLYCLEANFILES = perm_list.iwebadmin

edit = sed \
	-e 's,@mysql_prefix\@,$(mysql_prefix),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@docdir\@,$(docdir),g' \
	-e 's,@htmldir\@,$(htmldir),g' \
	-e 's,@htmllibdir\@,$(htmllibdir),g' \
	-e 's,@sys_libdir\@,$(sys_libdir),g' \
	-e 's,@cgibindir\@,$(cgibindir),g' \
	-e 's,@imagedir\@,$(imagedir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
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

iwebadmin.spec: iwebadmin.spec.in
	(cat $@.in;./catChangeLog) | $(edit) > $@
iwebadmin-rpmlintrc: iwebadmin-rpmlintrc.in
	$(edit) iwebadmin-rpmlintrc.in > $@
debian/Makefile: debian/Makefile.in conf-version conf-release
	$(edit) $@.in > $@
perm_list.iwebadmin: perm_list.iwebadmin.in
	$(edit) $@.in > $@
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
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@
	rm -rf                   $(DESTDIR)@htmllibdir@/html
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@/html 
	cp $(srcdir)/html/*.html $(DESTDIR)@htmllibdir@/html
	cp $(srcdir)/html/colortable $(DESTDIR)@htmllibdir@/html
	chmod 755                $(DESTDIR)@htmllibdir@/html
	chmod 644                $(DESTDIR)@htmllibdir@/html/*
	rm -rf                   $(DESTDIR)@htmllibdir@/lang
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@/lang 
	cp $(srcdir)/lang/??     $(DESTDIR)@htmllibdir@/lang
	cp $(srcdir)/lang/??-??  $(DESTDIR)@htmllibdir@/lang
	chmod 755                $(DESTDIR)@htmllibdir@/lang
	chmod 644                $(DESTDIR)@htmllibdir@/lang/*
	$(mkinstalldirs)         $(DESTDIR)@imagedir@
	cp $(srcdir)/images/*.png $(DESTDIR)@imagedir@

	if test `@idcommand@` = "0" ; then \
		echo "chown root $(DESTDIR)$(bindir)/iwebadmin"; \
		chown root $(DESTDIR)$(bindir)/iwebadmin; \
		echo "chmod 4555 $(DESTDIR)$(bindir)/iwebadmin"; \
		chmod 4555 $(DESTDIR)$(bindir)/iwebadmin; \
	fi
install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)@sysconfdir@
	$(mkinstalldirs)         $(DESTDIR)@docdir@
	$(INSTALL_DATA) perm_list.iwebadmin $(DESTDIR)@sysconfdir@/perm_list.iwebadmin  || exit 1
	$(INSTALL_DATA) README $(DESTDIR)@docdir@/README || exit 1
	$(INSTALL_DATA) README.hooks $(DESTDIR)@docdir@/README.hooks || exit 1
	$(INSTALL_DATA) COPYING $(DESTDIR)@docdir@/COPYING || exit 1
	$(INSTALL_DATA) AUTHORS $(DESTDIR)@docdir@/AUTHORS || exit 1