@SET_MAKE@
COFLAGS=-z+05:30

CFLAGS = -g -O4 -Wall -fPIC
bin_PROGRAMS = altermime
man_MANS = altermime.1
altermime_SOURCES = qpe.c strstack.c mime_alter.c ffget.c pldstr.c filename-filters.c logger.c \
					MIME_headers.c libmime-decoders.c boundary-stack.c \
					altermime.c
altermime_bindir=@prefix@/bin

doc_list=AUTHORS-altermime README-altermime.md LICENSE-altermime

boundary-stack.o: logger.h pldstr.h boundary-stack.h

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/$$f || exit 1 ;\
	done
AUTOMAKE_OPTIONS = foreign no-dependencies
