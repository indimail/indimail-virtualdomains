/*
 * $Log: autoturn_dir.c,v $
 * Revision 1.4  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2023-03-20 09:41:36+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2020-04-01 18:53:10+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
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
#include <getEnvConfig.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: autoturn_dir.c,v 1.4 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("autoturn_dir: out of memory", 0);
	_exit(111);
}

char           *
autoturn_dir(const char *domain)
{
	static stralloc filename = {0}, template = {0}, line = {0};
	char            inbuf[512];
	char           *ptr, *sysconfdir, *controldir;
	int             fd, match, i;
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
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die3sys(111, "autoturn_dir: read: ", filename.s, ": ");
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
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
