/*
 * $Log: host_in_locals.c,v $
 * Revision 1.3  2023-03-20 10:02:35+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2020-04-01 18:55:05+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:25:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: host_in_locals.c,v 1.3 2023-03-20 10:02:35+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <str.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <getEnvConfig.h>
#endif

static void
die_nomem()
{
	strerr_warn1("valias_insert: out of memory", 0);
	_exit(111);
}

int
host_in_locals(char *domain)
{
	static stralloc tmpbuf = {0}, line = {0};
	char           *ptr, *sysconfdir, *controldir;
	int             fd, match;
	char            inbuf[4096];
	struct substdio ssin;

	if(!domain || !*domain)
		return(0);
	else
	if (!str_diffn(domain, "localhost", 10))
		return (1);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/locals", 7) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/locals", 7) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "host_in_locals: open: ", tmpbuf.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for(;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("host_in_locals: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
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
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!str_diffn(domain, ptr, line.len - (ptr - line.s))) {
			close(fd);
			return (1);
		}
	}
	close(fd);
	return (0);
}
#endif
