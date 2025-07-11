/*
 * $Id: getFreeFS.c,v 1.4 2025-05-13 19:59:39+05:30 Cprogrammer Exp mbhangui $
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
#include <substdio.h>
#include <getln.h>
#include <strerr.h>
#include <str.h>
#include <open.h>
#include <error.h>
#include <getEnvConfig.h>
#endif
#include "get_local_hostid.h"
#include "pathToFilesystem.h"
#include "vfstab.h"
#include "dblock.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getFreeFS.c,v 1.4 2025-05-13 19:59:39+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc tmpbuf = {0};
static char    *sysconfdir;

static void
die_nomem()
{
	strerr_warn1("getFreeFS: out of memory", 0);
	_exit(111);
}

static char    *
getLastFstab()
{
#ifdef FILE_LOCKING
	int             lockfd;
#endif
	char           *ptr;
	int             fd, match;
	static stralloc line = {0};
	char            inbuf[4096];
	struct substdio ssin;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	if (!stralloc_copys(&tmpbuf, sysconfdir) ||
			!stralloc_catb(&tmpbuf, "/lastfstab", 10) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(tmpbuf.s, 1)) == -1)
		return ((char *) 0);
#endif
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_warn3("getFreeFS: open: ", tmpbuf.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return ((char *) 0);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn3("getFreeFS: read: ", tmpbuf.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		close(fd);
		return ((char *) 0);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, tmpbuf.s, 1);
#endif
	if (!line.len)
		return ((char *) 0);
	if (match) {
		line.len--;
		if (!line.len)
			return ((char *) 0);
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
		return ((char *) 0);
	return (ptr);
}

static int
putLastFstab(char *filesystem)
{
	int             fd;
	char            outbuf[4096];
	struct substdio ssout;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	if (!stralloc_copys(&tmpbuf, sysconfdir) ||
			!stralloc_catb(&tmpbuf, "/lastfstab", 10) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(tmpbuf.s, 1)) == -1)
		return (-1);
#endif
	if ((fd = open_trunc(tmpbuf.s)) == -1) {
		strerr_warn3("getFreeFS: open: ", tmpbuf.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (-1);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	if (substdio_puts(&ssout, filesystem) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout))
	{
		strerr_warn3("getFreeFS: write: ", tmpbuf.s, ": ", &strerr_sys);
		close(fd);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (-1);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, tmpbuf.s, 1);
#endif
	return (0);
}

char           *
getFreeFS()
{
	char           *lastSelectedFS, *tmpfstab, *local_hostid, *ptr;
	int             status, len;
	float           load, prev_load;
	long            cur_user, cur_size, max_user, max_size;
	static stralloc hostID = {0}, fileSystem = {0};

	if (!(lastSelectedFS = getLastFstab())) {
		strerr_warn1("getFreeFS: couldn't get last selected filesystem", 0);
		return ((char *) 0);
	}
	if (!(local_hostid = get_local_hostid())) {
		strerr_warn1("getFreeFS: Could not get local hostid: ", &strerr_sys);
		return ((char *) 0);
	} else {
		if (!stralloc_copys(&hostID, local_hostid) || !stralloc_0(&hostID))
			die_nomem();
	}
	len = str_len(lastSelectedFS);
	for (prev_load = -1;;) {
		if (!(tmpfstab = vfstab_select(&hostID, &status, &max_user, &cur_user, &max_size, &cur_size)))
			break;
		if (status == FS_OFFLINE)
			continue;
		if (lastSelectedFS && !str_diffn(tmpfstab, lastSelectedFS, len))
			continue;
		load = cur_size ? ((float) (cur_user * 1024 * 1024)/ (float) cur_size) : 0.0;
		if (load < prev_load || prev_load == -1.0) {
			if (!stralloc_copys(&fileSystem, tmpfstab) ||
					!stralloc_0(&fileSystem))
				die_nomem();
			fileSystem.len--;
		}
		prev_load = load;
	}
	if (fileSystem.len && !putLastFstab(fileSystem.s))
		return (fileSystem.s);
	getEnvConfigStr(&tmpfstab, "BASE_PATH", BASE_PATH);
	if (!(ptr = pathToFilesystem(tmpfstab)))
		return (tmpfstab);
	return (ptr);
}
/*
 * $Log: getFreeFS.c,v $
 * Revision 1.4  2025-05-13 19:59:39+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.3  2023-03-20 10:00:12+05:30  Cprogrammer
 * use SYSONFDIR env variable if set for lastfstab
 *
 * Revision 1.2  2020-04-01 18:59:42+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:25:28+05:30  Cprogrammer
 * Initial revision
 *
 */
