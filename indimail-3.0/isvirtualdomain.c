/*
 * $Log: isvirtualdomain.c,v $
 * Revision 1.3  2020-04-01 18:56:42+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-12-03 22:13:19+05:30  Cprogrammer
 * return 0 for all domains if virtualdomains control file is absent
 *
 * Revision 1.1  2019-04-18 08:25:39+05:30  Cprogrammer
 * Initial revision
 *
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
#include <open.h>
#include <error.h>
#include <strerr.h>
#include <substdio.h>
#include <getln.h>
#include <str.h>
#include <stralloc.h>
#include <getEnvConfig.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: isvirtualdomain.c,v 1.3 2020-04-01 18:56:42+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("is_alias_domain: out of memory", 0);
	_exit(111);
}

int
isvirtualdomain(char *domain)
{
	char           *sysconfdir, *controldir, *ptr;
	char            inbuf[512];
	int             fd, match, t;
	static stralloc tmp = {0}, line = {0};
	struct substdio ssin;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmp, controldir) || !stralloc_catb(&tmp, "/virtualdomains", 15) ||
				!stralloc_0(&tmp))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&tmp, sysconfdir) ||
				!stralloc_append(&tmp, "/") ||
				!stralloc_cats(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/virtualdomains", 15) ||
				!stralloc_0(&tmp))
			die_nomem();
	}
	if ((fd = open_read(tmp.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "is_alias_domain: ", tmp.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "is_alias_domain: read: ", tmp.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		match = str_chr(ptr, ':');
		if (ptr[match])
			ptr[match] = 0;
		if (!(str_diff(ptr, domain))) {
			close(fd);
			return (1);
		}
	}
	close(fd);
	return (0);
}
