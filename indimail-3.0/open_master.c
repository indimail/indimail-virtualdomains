/*
 * $Log: open_master.c,v $
 * Revision 1.1  2019-04-14 18:34:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: open_master.c,v 1.1 2019-04-14 18:34:39+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"
#include "open.h"
#include "error.h"
#include "substdio.h"
#include "getln.h"
#include "findhost.h"

struct substdio ssin;
static char     inbuf[512];
static stralloc line = {0};

static void
die_nomem()
{
	strerr_warn1("open_master: out of memory", 0);
	_exit(111);
}

int
open_master()
{
	char           *ptr, *sysconfdir, *controldir;
	static stralloc host_path = {0};
	int             fd, match, t;

	if ((ptr = (char *) env_get("MASTER_HOST")) != (char *) 0)
		return (open_central_db(ptr));
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.master", 12) ||
			!stralloc_0(&host_path))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&host_path, sysconfdir) ||
			!stralloc_append(&host_path, "/") ||
			!stralloc_cats(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.master", 12) ||
			!stralloc_0(&host_path))
			die_nomem();
	}
	if ((fd = open_read(host_path.s)) == -1) {
		if (errno == error_noent) {
			return (2);
		} else
			strerr_die3sys(111, "open_master: open: ", host_path.s, ": ");
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		t = errno;
		close(fd);
		errno = t;
		strerr_die3sys(111, "read: ", host_path.s, ": ");
	}
	close(fd);
	if (line.len == 0) {
		strerr_die3sys(111, "open_master: ", host_path.s, "incomplete line: ");
	}
	if (match)
		line.s[line.len - 1] = 0; /*- remove newline */
	return (open_central_db(line.s));
}
#endif
