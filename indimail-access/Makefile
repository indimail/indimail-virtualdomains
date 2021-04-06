COFLAGS=-z+05:30
prefix=/usr
indimaildir=/var/indimail
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
servicedir=/service
logdir=/var/log/svc
mandir=/usr/share/man
defaultDomain=indimail.org
version := $(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null||(echo "1.1" && echo 1.1>conf-release))
email := $(shell cat conf-email)
courier_version=$(shell cat ../courier-imap-x/conf-version)
fetchmail_version=$(shell cat ../fetchmail-x/conf-version)
DATE=$(shell date)
# if you change edit, change it in debian/Makefile.in too
edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@defaultDomain\@,$(defaultDomain),g' \
	-e 's,@courier_version\@,$(courier_version),g' \
	-e 's,@fetchmail_version\@,$(fetchmail_version),g' \
	-e 's,@DATE\@,$(DATE),g'

all: indimail-access.spec indimail-access.changes indimail-access-rpmlintrc \
indimail-access-permissions.easy indimail-access-permissions.secure \
indimail-access-permissions.paranoid perm_list.imap debian/Makefile \
PKGBUILD

clean:
	/bin/rm -f indimail-access.spec indimail-access.changes \
	indimail-access-rpmlintrc indimail-access-permissions.easy \
	indimail-access-permissions.secure indimail-access-permissions.paranoid \
	perm_list.imap debian/Makefile

indimail-access.spec: indimail-access.spec.in \
conf-version conf-release doc/ChangeLog conf-email \
../fetchmail-x/conf-version ../courier-imap-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
indimail-access.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release conf-email \
../fetchmail-x/conf-version ../courier-imap-x/conf-version \
indimail-access.changes
	cat $@.in | $(edit) > $@
indimail-access-rpmlintrc: indimail-access-rpmlintrc.in
	$(edit) $@.in > $@
indimail-access-permissions.easy: indimail-access-permissions.easy.in
	$(edit) $@.in > $@
indimail-access-permissions.secure: indimail-access-permissions.secure.in
	$(edit) $@.in > $@
indimail-access-permissions.paranoid: indimail-access-permissions.paranoid.in
	$(edit) $@.in > $@
perm_list.imap: perm_list.in
	$(edit) perm_list.in > $@
debian/Makefile: debian/Makefile.in ../fetchmail-x/conf-version \
../courier-imap-x/conf-version conf-release
	$(edit) $@.in > $@
