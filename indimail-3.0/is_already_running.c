/*
 * $Log: is_already_running.c,v $
 * Revision 1.1  2019-04-18 08:21:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <scan.h>
#include <fmt.h>
#include <error.h>
#endif

#ifndef lint
static char     sccsid[] = "$Id: is_already_running.c,v 1.1 2019-04-18 08:21:41+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("is_already_running: out of memory", 0);
	_exit(111);
}

/*
 * 0 - not running
 * 1 - already running
 */
int
is_already_running(char *pgname)
{
	static stralloc filename = {0}, line = {0};
	int             pid, fd, match, i;
	char            inbuf[FMT_ULONG], outbuf[FMT_ULONG], strnum[FMT_ULONG];
	struct substdio ssin, ssout;

	if (!stralloc_copyb(&filename, "/tmp/", 5) ||
			!stralloc_cats(&filename, pgname) ||
			!stralloc_catb(&filename, ".PID", 4) ||
			!stralloc_0(&filename))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "is_already_running: open: ", filename.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		i = errno;
		close(fd);
		errno = i;
		strerr_die3sys(111, "is_already_running: read: ", filename.s, ": ");
	}
	close(fd);
	if (!line.len)
		strerr_die3x(111, "is_already_running: ", filename.s, ": incomplete line");
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	} else
	if (!stralloc_0(&line))
		die_nomem();
	scan_ulong(line.s, (unsigned long *) &pid);
	if (pid && !kill(pid, 0))
		return (pid);
	if ((fd = open_trunc(filename.s)) == -1)
		strerr_die3sys(111, "is_already_running: open_trunc: ", filename.s, ": ");
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	strnum[i = fmt_ulonglong(strnum, getpid())] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}
