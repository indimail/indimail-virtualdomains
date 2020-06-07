COFLAGS=-z+05:30
prefix=/usr
indimaildir=/var/indimail
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
libdir=/usr/lib
servicedir=/service
logdir=/var/log/svc
mandir=/usr/share/man
defaultDomain=indimail.org
version=$(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null||echo "1.1")
courier_version=$(shell cat ../courier-imap-x/conf-version)
fetchmail_version=$(shell cat ../fetchmail-x/conf-version)
DATE=$(shell date)
# if you change edit, change it in debian/Makefile.in too
edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@courier_version\@,$(courier_version),g' \
	-e 's,@fetchmail_version\@,$(fetchmail_version),g' \
	-e 's,@DATE\@,$(DATE),g'

all: indimail-access.spec indimail-access-rpmlintrc \
	indimail-access-permissions.easy indimail-access-permissions.secure \
	indimail-access-permissions.paranoid perm_list.imap debian/Makefile

indimail-access.spec: indimail-access.spec.in catChangeLog Makefile conf-version conf-release
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
indimail-access-rpmlintrc: indimail-access-rpmlintrc.in Makefile
	$(edit) $@ > $@
indimail-access-permissions.easy: indimail-access-permissions.easy.in Makefile
	$(edit) $@ > $@
indimail-access-permissions.secure: indimail-access-permissions.secure.in Makefile
	$(edit) $@ > $@
indimail-access-permissions.paranoid: indimail-access-permissions.paranoid.in Makefile
	$(edit) $@ > $@
perm_list.imap: perm_list.in
	$(edit) perm_list.in > $@
debian/Makefile: debian/Makefile.in
	$(edit) $@.in > $@
