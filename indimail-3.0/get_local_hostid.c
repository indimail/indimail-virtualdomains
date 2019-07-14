/*
 * $Log: get_local_hostid.c,v $
 * Revision 1.2  2019-07-04 10:04:40+05:30  Cprogrammer
 * collapsed multiple if statements
 *
 * Revision 1.1  2019-04-14 18:32:29+05:30  Cprogrammer
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
#include <open.h>
#include <stralloc.h>
#include <fmt.h>
#include <error.h>
#include <getln.h>
#include <strerr.h>
#endif
#include "getEnvConfig.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_local_hostid.c,v 1.2 2019-07-04 10:04:40+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc filename = { 0 };
static stralloc line = { 0 };
static char     strnum[FMT_ULONG];

static void
die_nomem(char *prefix)
{
	strerr_warn2(prefix, ": out of memory", 0);
	_exit(111);
}

char           *
get_local_hostid()
{
	char            inbuf[512];
	char           *sysconfdir, *controldir;
	int             fd;
	int             match, t;
	long            hostidno;
	substdio        ssin;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) || !stralloc_catb(&filename, "/hostid", 7) ||
				!stralloc_0(&filename))
			die_nomem("get_local_hostid");
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&filename, sysconfdir))
			die_nomem("get_local_hostid");
		else
		if (!stralloc_append(&filename, "/") || !stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/hostid", 7))
			die_nomem("get_local_hostid");
	}
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "get_local_hostid", filename.s, ": ");
		if ((hostidno = gethostid()) == -1)
			return ((char *) 0);
		t = fmt_xlong(strnum, hostidno);
		if (!stralloc_copyb(&line, strnum, t))
			die_nomem("get_local_hostid");
		else
		if (!stralloc_0(&line))
			die_nomem("get_local_hostid");
		return (line.s);
	} else
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "get_local_hostid: read: ", filename.s, ": ");
		}
		if (!match)
			break;
		else {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		return (line.s);
	}
	line.s[line.len] = 0;
	return (line.s);
}
