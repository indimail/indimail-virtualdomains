@SET_MAKE@
COFLAGS=-z+05:30

COOKIES=art ascii-art computers cookie definitions drugs education ethnic \
	food fortunes goedel humorists kids law linuxcookie literature \
	love magic medicine men-women miscellaneous news people pets \
	platitudes politics riddles science songs-poems sports \
	startrek translate-me wisdom work zippy kernelnewbies tao \
	rj bofh-excuses perl debian knghtbrd linux paradoxum chanakya \
	thirukkural paul-graham joel-on-software osp vedas-1 \
	vedas-2 age
OCOOKIES=drugsO misandry privates sex astrology ethnicO \
	miscellaneousO racism songs-poemsO black-humor hphobia misogyny \
	religion vulgarity definitionsO limerick politicsO riddlesO \
	atheism fortunesO linuxO

COMMONSOURCES= fortune.c randstr.c rot.c strfile.c unstr.c

fortune_SOURCES = fortune.c

fortunebindir=@prefix@/bin
fortunesbindir=@prefix@/sbin
fortunedatadir=@datarootdir@/fortunes
#fortunedata_INSTALL=./copy_fortune

fortunebin_PROGRAMS = fortune randstr
fortunesbin_PROGRAMS = strfile unstr rot
fortunedata_DATA=cookies ocookies
noinst_SCRIPTS = copy_fortune
man1_MANS=fortune.man randstr.man strfile.man unstr.man rot.man

doc_list=AUTHORS-fortune README-fortune.md COPYING-fortune

MOSTLYCLEANFILES=stamp-cookies stamp-ocookies datfiles/*.dat

fortune.o: fortune.c strfile.h config.h
cookies: stamp-cookies
ocookies: stamp-ocookies
copy_fortune: copy_fortune.in
	$(edit) $@.in > $@
mostlyclean-local:
	cd datfiles;rm -f $(OCOOKIES)

edit = sed \
	-e 's,@COOKIES\@,$(COOKIES),g' \
	-e 's,@OCOOKIES\@,$(OCOOKIES),g'

stamp-cookies: datfiles strfile
	for i in $(COOKIES) ; do ./strfile datfiles/$$i >/dev/null || exit 1 ; done; touch stamp-cookies
stamp-ocookies: datfiles/off/unrotated
	if [ " $(ENABLE_OFFENSIVE)" = " 1" ] ; then \
	for i in $(OCOOKIES) ; do rot < datfiles/off/unrotated/$$i > datfiles/$$i && ./strfile -x datfiles/$$i >/dev/null || exit 1 ; done ; touch stamp-ocookies;fi

install-data-hook: copy_fortune
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/$$f || exit 1 ;\
	done
AUTOMAKE_OPTIONS = foreign no-dependencies
