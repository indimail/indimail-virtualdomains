/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <open.h>
#include <getln.h>
#include <error.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: count_rcpthosts.c,v 1.3 2023-03-20 09:51:06+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("count_rcpthosts: out of memory", 0);
	_exit(111);
}

/*
 * count the lines in rcpthosts
 */

int
count_rcpthosts()
{
	static stralloc line = { 0 }, filename = {0};
	char           *sysconfdir, *controldir, *ptr;
	int             fd, match;
	register int    count;
	char            inbuf[4096];
	struct substdio ssin;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "rcpthosts", 9) || !stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "rcpthosts", 9) || !stralloc_0(&filename))
			die_nomem();
	}
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "count_rcpthosts: open: ", filename.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (count = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("count_rcpthosts: read: ", filename.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (line.len == 0)
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
		count++;
	}
	close(fd);
	return (count);
}
/*
 * $Log: count_rcpthosts.c,v $
 * Revision 1.3  2023-03-20 09:51:06+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2020-04-01 18:53:40+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:22:40+05:30  Cprogrammer
 * Initial revision
 *
 */
