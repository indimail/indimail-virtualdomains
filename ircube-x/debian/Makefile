prefix=/usr
version=$(shell cat ../conf-version)
release=$(shell cat ../conf-release)
#

all: rules debian.tar.gz dummy.tar.gz ircube.dsc changelog

clean:
	/bin/rm -f rules debian.tar.gz dummy.tar.gz

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@prefix\@,$(prefix),g'
changelog: changelog.in ../conf-version ../conf-release
	(cat $@.in; echo ""; \
	echo " -- Manvendra Bhangui <webmail@indimail.org>  `date +'%a, %d %b %Y %H:%M:%S %z'`" ) \
	| $(edit) > $@
ircube.dsc: ircube.dsc.in ../conf-version ../conf-release
	$(edit) $@.in > $@

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat ircube.install Makefile ../conf-version ../conf-release
	tar cf - copyright rules control changelog compat ircube.install | gzip > $@
