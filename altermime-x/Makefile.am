@SET_MAKE@
# AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = altermime
man_MANS = altermime.1
altermime_SOURCES = qpe.c strstack.c mime_alter.c ffget.c pldstr.c filename-filters.c logger.c \
					MIME_headers.c libmime-decoders.c boundary-stack.c \
					altermime.c
include_HEADERS = boundary-stack.h config.h ffget.h filename-filters.h \
	libmime-decoders.h logger.h mime_alter.h MIME_headers.h pldstr.h \
	qpe.h strstack.h
altermime_bindir=@prefix@/bin

doc_list=AUTHORS-altermime README-altermime.md LICENSE-altermime

boundary-stack.o: logger.h pldstr.h boundary-stack.h

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)$(datarootdir)/doc/$$f || exit 1 ;\
	done
