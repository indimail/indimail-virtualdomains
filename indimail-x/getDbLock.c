/*
 * $Log: getDbLock.c,v $
 * Revision 1.1  2019-04-20 09:18:06+05:30  Cprogrammer
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
#include <strerr.h>
#include <fmt.h>
#include <open.h>
#include <stralloc.h>
#endif
#include "lockfile.h"

#ifndef	lint
static char     sccsid[] = "$Id: getDbLock.c,v 1.1 2019-04-20 09:18:06+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem(char *str)
{
	strerr_warn2(str, ": out of memory", 0);
	_exit(111);
}

int
delDbLock(int lockfd, char *filename, char proj)
{
#ifdef FILE_LOCKING
	if (RemoveLock(filename, proj) == -1) {
		ReleaseLock(lockfd);
		return (-1);
	}
	if (ReleaseLock(lockfd) == -1)
		return (-1);
#endif
	return (0);
}

int
getDbLock(char *filename, char proj)
{
#ifdef FILE_LOCKING
	int             lockfd;
	char            strnum[FMT_ULONG];

	if ((lockfd = lockcreate(filename, proj)) == -1) {
		strnum[fmt_uint(strnum, proj)] = 0;
		strerr_warn5("getDbLock: lockcreate: ", filename, ".", strnum, ": ", &strerr_sys);
		return (-1);
	} else
	if (get_write_lock(lockfd) == -1) {
		strnum[fmt_uint(strnum, proj)] = 0;
		strerr_warn5("getDbLock: get_write_lock: ", filename, ".", strnum, ": ", &strerr_sys);
		delDbLock(lockfd, filename, proj);
		return (-1);
	}
	return (lockfd);
#else
	return (0);
#endif
}

int
readPidLock(char *filename, char proj)
{
	static stralloc fname = {0};
	int             fd;
	pid_t           pid;
	char            strnum[FMT_ULONG];

	if (!stralloc_copys(&fname, filename) ||
			!stralloc_catb(&fname, ".pre.", 5) ||
			!stralloc_catb(&fname, strnum, fmt_uint(strnum, proj)) ||
			!stralloc_0(&fname))
		die_nomem("readPidLock");
	if ((fd = open_read(fname.s)) == -1)
		return (-1);
	if (read(fd, (char *) &pid, sizeof(pid_t)) == -1) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (pid);
}
