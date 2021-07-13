prefix=/usr
version=$(shell cat ../conf-version)
release=$(shell cat ../conf-release)
email=$(shell cat ../conf-email)
#

all: rules debian.tar.gz dummy.tar.gz ircube.dsc changelog

clean:
	/bin/rm -f rules debian.tar.gz dummy.tar.gz

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@prefix\@,$(prefix),g'
changelog: ../doc/ChangeLog Makefile ../conf-version ../conf-release ../conf-email
	../catChangeLog --debian --name=ircube --state=stable \
		--urgency=low ../doc/ChangeLog > $@
control: control.in ../conf-email
	$(edit) $@.in > $@
copyright: copyright.in ../conf-email
	$(edit) $@.in > $@
ircube.dsc: ircube.dsc.in ../conf-version ../conf-release ../conf-email
	$(edit) $@.in > $@

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat ircube.install Makefile ../conf-version ../conf-release
	tar cf - copyright rules control changelog compat ircube.install | gzip > $@
