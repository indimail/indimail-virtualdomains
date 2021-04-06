sysconfdir=/etc/indimail
mandir=/usr/share/man
prefix=/usr
version=$(shell cat ../conf-version)
release=$(shell cat ../conf-release)
email=$(shell cat ../conf-email)
#

all: rules debian.tar.gz dummy.tar.gz  \
	bogofilter-wordlist.dsc

clean:
	/bin/rm -f rules *.dsc debian.tar.gz dummy.tar.gz README

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@prefix\@,$(prefix),g'

rules: rules.in Makefile
	$(edit) $@.in > $@; chmod +x $@;
bogofilter-wordlist.dsc: bogofilter-wordlist.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
changelog: ../doc/ChangeLog ../conf-version ../conf-release ../conf-email
	../catChangeLog --debian --name=bogofilter-wordlist --state=unstable \
		--urgency=low ../doc/ChangeLog > $@
control: control.in ../conf-email
	$(edit) $@.in > $@
copyright: copyright.in ../conf-email
	$(edit) $@.in > $@

dummy.tar.gz:
	echo "dummy file" > README; tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat bogofilter-wordlist.install
	tar cf - copyright rules control changelog compat bogofilter-wordlist.install|gzip > $@
