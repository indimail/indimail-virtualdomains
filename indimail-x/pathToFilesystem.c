/*
 * $Log: pathToFilesystem.c,v $
 * Revision 1.5  2022-05-10 20:04:58+05:30  Cprogrammer
 * replaced Dirname with qdirname from libqmail
 *
 * Revision 1.4  2020-09-21 07:55:34+05:30  Cprogrammer
 * fixed unterminated stralloc variable
 *
 * Revision 1.2  2020-05-04 10:39:45+05:30  Cprogrammer
 * use /proc/mounts, /proc/self/mounts for docker containers
 *
 * Revision 1.1  2019-04-18 08:31:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef linux
#include <mntent.h>
#elif defined(sun)
#include <sys/types.h>
#include <sys/mnttab.h>
#elif defined(DARWIN) || defined(FREEBSD)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#include <qdirname.h>
#endif
#include "getactualpath.h"

#ifndef	lint
static char     sccsid[] = "$Id: pathToFilesystem.c,v 1.5 2022-05-10 20:04:58+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("pathToFilesystem: out of memory", 0);
	_exit(111);
}

char           *
pathToFilesystem(char *path)
{
	char           *ptr;
	int             pathlen, len;
	static stralloc tmpbuf = {0}, _path = {0};
#ifdef linux
	FILE           *fp;
	struct mntent  *mntptr;
#elif defined(sun)
#define mnt_dir    mnt_mountp
	FILE           *fp;
	struct mnttab   _MntTab;
	struct mnttab  *mntptr = &_MntTab;
#elif defined(DARWIN) || defined(FREEBSD)
	int             num;
	struct statfs  *mntinf;
#endif

	if (!path || !*path)
		return ((char *) 0);
	if (_path.len && !str_diffn(path, _path.s, _path.len + 1))
		return (tmpbuf.s);
	/*
	 * if directory does not exists, find parent
	 * directory recursively
	 */
	for (ptr = path; access(ptr, F_OK);) {
		if (!(ptr = qdirname(ptr)))
			break;
		if (!access(ptr, F_OK))
			break;
	}
	/*- Resolve links and Find the actual path */
	if (!(ptr = getactualpath(ptr)))
		return ((char *) 0);
#if defined(DARWIN) || defined(FREEBSD)
	if (!(num = getmntinfo(&mntinf, MNT_WAIT)))
		return ((char *) 0);
	for (tmpbuf.len = 0, pathlen = 0;num--;) {
		if (str_str(ptr, mntinf->f_mntonname)) {
			if ((len = str_len(mntinf->f_mntonname)) > pathlen) {
				if (!stralloc_copys(&tmpbuf, mntinf->f_mntonname) || !stralloc_0(&tmpbuf))
					die_nomem();
				pathlen = len;
			}
		}
		mntinf++;
	}
#else /*- DARWIN || FREEBSD */
	fp = (FILE *) 0;
#ifdef linux
	if (access("/etc/mtab", F_OK)) {
		if (!access("/proc/self/mounts", F_OK))
			fp = setmntent("/proc/self/mounts", "r");
		else
		if (!access("/proc/mounts", F_OK))
			fp = setmntent("/proc/mounts", "r");
		else {
			errno = 2;
			strerr_warn1("pathToFilesystem: /etc/mtab, /proc/mounts, /proc/self/mounts", &strerr_sys);
			return ((char *) 0);
		}
	} else
		fp = setmntent("/etc/mtab", "r");
#elif defined(sun)
	if (!access("/etc/mnttab", F_OK))
		fp = fopen("/etc/mnttab", "r");
#endif
	if (!fp)
		return ((char *) 0);
	for (tmpbuf.len = 0, pathlen = 0;;) {
#ifdef linux
		if (!(mntptr = getmntent(fp)))
#elif defined(sun)
		if (getmntent(fp, mntptr))
#endif
			break;
		if (str_str(ptr, mntptr->mnt_dir)) {
			if ((len = str_len(mntptr->mnt_dir)) > pathlen) {
				if (!stralloc_copys(&tmpbuf, mntptr->mnt_dir) || !stralloc_0(&tmpbuf))
					die_nomem();
				tmpbuf.len--;
				pathlen = len;
			}
		}
	}
	fclose(fp);
#endif /*- #ifdef DARWIN */
	if (!stralloc_copys(&_path, path) || !stralloc_0(&_path))
		die_nomem();
	_path.len--;
	return (tmpbuf.s);
}
