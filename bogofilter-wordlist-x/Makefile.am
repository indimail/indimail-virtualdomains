@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
version=@VERSION@
release=@release@

SYSCONFDIR = @sysconfdir@
sysconf_DATA = wordlist.db

noinst_DATA = bogofilter-wordlist.spec

all-local: bogofilter-wordlist.spec
install-data-local: wordlist.db

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@sysconfdir@ || exit 1

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(version),g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's,@prefix\@,$(prefix),g'

bogofilter-wordlist.spec: bogofilter-wordlist.spec.in conf-version conf-release
	$(edit) $@.in > $@

wordlist.db: training/easy_ham training/easy_ham_2 training/hard_ham \
training/spam training/spam_2
	./download.sh && ./training.sh