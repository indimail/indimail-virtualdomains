/*
 * $Log: autoturn_dir.c,v $
 * Revision 1.1  2019-04-18 08:25:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <open.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#include <stralloc.h>
#endif
#include "getEnvConfig.h"

#ifndef	lint
static char     sccsid[] = "$Id: autoturn_dir.c,v 1.1 2019-04-18 08:25:27+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("autoturn_dir: out of memory", 0);
	_exit(111);
}

char           *
autoturn_dir(char *domain)
{
	static stralloc filename = {0}, template = {0}, line = {0};
	char            inbuf[512];
	char           *ptr, *sysconfdir, *controldir;
	int             fd, match, t, i;
	substdio        ssin;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/virtualdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/virtualdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	if (!stralloc_copys(&template, domain) || !stralloc_append(&template, ":"))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1)
		return ((char *) 0);
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "autoturn_dir: read: ", filename.s, ": ");
		}
		if (!match)
			break;
		else {
			line.len--;
			line.s[line.len] = 0;
		}
		i = str_chr(line.s, '#');
		line.s[i] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!str_diffn(line.s, template.s, template.len)) {
			close(fd);
			return (line.s + template.len + 9);
		}
	}
	close(fd);
	return ((char *) 0);
}
