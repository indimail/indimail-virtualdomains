/*
 * $Log: update_local_hostid.c,v $
 * Revision 1.1  2019-04-18 08:33:47+05:30  Cprogrammer
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
#include <error.h>
#include <strerr.h>
#include <substdio.h>
#endif
#include "getEnvConfig.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_local_hostid.c,v 1.1 2019-04-18 08:33:47+05:30 Cprogrammer Exp mbhangui $";
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
		if (!stralloc_copys(&filename, controldir))
			die_nomem("update_local_hostid");
		else
		if (!stralloc_catb(&filename, "/hostid", 7))
			die_nomem("update_local_hostid");
		else
		if (!stralloc_0(&filename))
			die_nomem("update_local_hostid");
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&filename, sysconfdir))
			die_nomem("update_local_hostid");
		else
		if (!stralloc_append(&filename, "/"))
			die_nomem("update_local_hostid");
		else
		if (!stralloc_cats(&filename, controldir))
			die_nomem("update_local_hostid");
		else
		if (!stralloc_catb(&filename, "/hostid", 7))
			die_nomem("update_local_hostid");
	}
	if ((fd = open_trunc(filename.s)) == -1)
		strerr_die3sys(111, "update_local_hostid: open: ", filename.s, ": ");
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_puts(&ssout, hostid) || substdio_flush(&ssout))
		strerr_die3sys(111, "update_local_hostid: write: ", filename.s, ": ");
	return (close(fd));
}
