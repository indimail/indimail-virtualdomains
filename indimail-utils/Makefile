COFLAGS=-z+05:30
prefix=/usr
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
mandir=/usr/share/man
version := $(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null||(echo "1.1" && echo 1.1>conf-release))
email := $(shell cat conf-email)
altermime_version=$(shell cat ../altermime-x/conf-version)
ripmime_version=$(shell cat ../ripmime-x/conf-version)
fortune_version=$(shell cat ../fortune-x/conf-version)
mpack_version=$(shell cat ../mpack-x/conf-version)
flash_version=$(shell cat ../flash-x/conf-version)
DATE=$(shell date)
# if you change edit, change it in debian/Makefile.in too
edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@altermime_version\@,$(altermime_version),g' \
	-e 's,@ripmime_version\@,$(ripmime_version),g' \
	-e 's,@fortune_version\@,$(fortune_version),g' \
	-e 's,@flash_version\@,$(flash_version),g' \
	-e 's,@mpack_version\@,$(mpack_version),g' \
	-e 's,@DATE\@,$(DATE),g'

all: indimail-utils.spec indimail-utils.changes debian/Makefile PKGBUILD

indimail-utils.spec: indimail-utils.spec.in catChangeLog conf-email \
conf-version conf-release ../altermime-x/conf-version ../ripmime-x/conf-version \
../mpack-x/conf-version ../flash-x/conf-version ../fortune-x/conf-version doc/ChangeLog
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
indimail-utils.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-email conf-version conf-release \
../altermime-x/conf-version ../ripmime-x/conf-version \
../mpack-x/conf-version ../flash-x/conf-version ../fortune-x/conf-version \
indimail-utils.changes
	cat $@.in | $(edit) > $@
debian/Makefile: debian/Makefile.in ../altermime-x/conf-version ../ripmime-x/conf-version \
../mpack-x/conf-version ../flash-x/conf-version ../fortune-x/conf-version
	$(edit) $@.in > $@
