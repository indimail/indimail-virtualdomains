/*
 * $Id: update_local_hostid.c,v 1.4 2025-05-13 20:35:38+05:30 Cprogrammer Exp mbhangui $
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
#include <error.h>
#include <strerr.h>
#include <substdio.h>
#include <getEnvConfig.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: update_local_hostid.c,v 1.4 2025-05-13 20:35:38+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc filename = { 0 };

static void
die_nomem(char *prefix)
{
	strerr_warn2(prefix, ": out of memory", 0);
	_exit(111);
}

int
update_local_hostid(char *hostid)
{
	char            outbuf[512];
	char           *sysconfdir, *controldir;
	int             fd;
	substdio        ssout;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) || !stralloc_catb(&filename, "/hostid", 7) ||
				!stralloc_0(&filename))
			die_nomem("update_local_hostid");
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&filename, sysconfdir) || !stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) || !stralloc_catb(&filename, "/hostid", 7))
			die_nomem("update_local_hostid");
	}
	if ((fd = open_trunc(filename.s)) == -1)
		strerr_die3sys(111, "update_local_hostid: open: ", filename.s, ": ");
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	if (substdio_puts(&ssout, hostid) || substdio_flush(&ssout))
		strerr_die3sys(111, "update_local_hostid: write: ", filename.s, ": ");
	return (close(fd));
}
/*
 * $Log: update_local_hostid.c,v $
 * Revision 1.4  2025-05-13 20:35:38+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.3  2020-04-01 18:58:12+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-07-04 10:08:39+05:30  Cprogrammer
 * collapsed multiple if statements
 *
 * Revision 1.1  2019-04-18 08:33:47+05:30  Cprogrammer
 * Initial revision
 *
 */
