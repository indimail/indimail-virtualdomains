COFLAGS=-z+05:30
prefix=/usr
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
mandir=/usr/share/man
logdir=/var/log/svc
pam_confdir=/etc/pam.d
servicedir=/service
version := $(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null||(echo "1.1" && echo 1.1>conf-releae))
nssd_version=$(shell cat ../nssd-x/conf-version)
pam_multi_version=$(shell cat ../pam-multi-x/conf-version)
email := $(shell cat conf-email)
DATE=$(shell date)
# if you change edit, change it in debian/Makefile.in too
edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@pam_confdir\@,$(pam_confdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@nssd_version\@,$(nssd_version),g' \
	-e 's,@pam_multi_version\@,$(pam_multi_version),g' \
	-e 's,@DATE\@,$(DATE),g'

all: indimail-auth.spec indimail-auth.changes debian/Makefile PKGBUILD \
perm_list.auth

indimail-auth.spec: indimail-auth.spec.in \
conf-version conf-release doc/ChangeLog conf-email \
../nssd-x/conf-version ../pam-multi-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
indimail-auth.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release \
conf-email ../nssd-x/conf-version ../pam-multi-x/conf-version \
indimail-auth.changes
	cat $@.in | $(edit) > $@
debian/Makefile: debian/Makefile.in conf-version conf-release \
../nssd-x/conf-version ../pam-multi-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
	$(edit) $@.in > $@
perm_list.auth: perm_list.in
	$(edit) perm_list.in > $@
clean:
	/bin/rm -f indimail-auth.spec indimail-auth.changes PKGBUILD \
	debian/Makefile perm_list.auth

