sysconfdir=/etc/indimail
mandir=/usr/share/man
prefix=/usr
version=$(shell cat ../conf-version)
release=$(shell cat ../conf-release)
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
	-e 's,@prefix\@,$(prefix),g'

rules: rules.in Makefile
	$(edit) $@.in > $@; chmod +x $@;
bogofilter-wordlist.dsc: bogofilter-wordlist.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
changelog: changelog.in ../conf-version ../conf-release Makefile
	(cat $@.in; echo ""; \
	echo " -- Manvendra Bhangui <indimail-spamfilter@indimail.org>  `date +'%a, %d %b %Y %H:%M:%S %z'`" ) \
	| $(edit) > $@

dummy.tar.gz:
	echo "dummy file" > README; tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat bogofilter-wordlist.install
	tar cf - copyright rules control changelog compat bogofilter-wordlist.install|gzip > $@
