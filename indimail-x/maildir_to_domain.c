/*
 * $Log: maildir_to_domain.c,v $
 * Revision 1.2  2019-04-21 16:15:54+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:27:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <getln.h>
#include <substdio.h>
#include <open.h>
#include <error.h>
#include <strerr.h>
#include <str.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: maildir_to_domain.c,v 1.2 2019-04-21 16:15:54+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("maildir_to_domain: out of memory", 0);
	_exit(111);
}

char           *
maildir_to_domain(char *maildir)
{
	static stralloc line = {0}, filename = {0};
	int             fd, len, match;
	struct substdio ssin;
	char            inbuf[512];

	len = str_len(maildir);
	if (maildir[len - 1] == '/')
		len--;
	if (!stralloc_catb(&filename, maildir, len) ||
			!stralloc_catb(&filename, "/domain", 7) ||
			!stralloc_0(&filename))
		die_nomem();
	if((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_warn3("maildir_to_domain: ", filename.s, ": ", &strerr_sys);
		return ((char *) 0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn3("maildir_to_domain: read: ", filename.s, ": ", &strerr_sys);
		close(fd);
		return ((char *) 0);
	}
	if (!match && line.len == 0) {
		close(fd);
		return ((char *) 0);
	}
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	}
	close(fd);
	return (line.s);
}
