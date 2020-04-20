/*
 * $Log: makeseekable.c,v $
 * Revision 1.2  2019-07-04 10:07:41+05:30  Cprogrammer
 * collapsed multiple if statements
 *
 * Revision 1.1  2019-04-18 08:31:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: makeseekable.c,v 1.2 2019-07-04 10:07:41+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef MAKE_SEEKABLE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#endif

static void
die_nomem()
{
	strerr_warn1("makeseekable: out of memory", 0);
	_exit(111);
}

int
makeseekable(int seekfd)
{
	char            inbuf[8192], outbuf[8192], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = { 0 };
	struct substdio ssin;
	struct substdio ssout;
	int             fd;

	if (!lseek(seekfd, 0, SEEK_SET))
		return (0);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir) || !stralloc_cats(&tmpFile, "/vdeliverXXXXXX") ||
			!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())) ||
			!stralloc_0(&tmpFile))
		die_nomem();
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1) {
		strerr_warn3("makeseekable: read error: ", tmpFile.s, ": ", &strerr_sys);
		_exit (111);
	}
	unlink(tmpFile.s);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof (inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		strerr_warn1("makeseekable: read error: ", &strerr_sys);
		close(fd);
		_exit(111);
	case -3: /*- write error */
		strerr_warn1("makeseekable: write error: ", &strerr_sys);
		close(fd);
		_exit(111);
	}
	if (substdio_flush(&ssout) == -1) {
		strerr_warn1("makeseekable: write error: ", &strerr_sys);
		close(fd);
		_exit(111);
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			strerr_warn1("makeseekable: dup2 error: ", &strerr_sys);
			close(fd);
			_exit(111);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0) {
		strerr_warn1("makeseekable: lseek error: ", &strerr_sys);
		close(seekfd);
		_exit(111);
	}
	return (0);
}
#endif
