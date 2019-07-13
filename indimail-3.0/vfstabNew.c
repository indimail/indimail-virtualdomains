/*
 * $Log: vfstabNew.c,v $
 * Revision 1.2  2019-04-22 23:19:16+05:30  Cprogrammer
 * added stdlib.h header
 *
 * Revision 1.1  2019-04-18 08:34:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#define XOPEN_SOURCE = 600
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/socket.h>
#include <mysql.h>
#include <mysqld_error.h>
#if defined(sun)
#include <sys/types.h>
#include <sys/statvfs.h>
#elif defined(DARWIN)
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "variables.h"
#include "getEnvConfig.h"
#include "get_local_ip.h"
#include "pathToFilesystem.h"
#include "vfstab.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstabNew.c,v 1.2 2019-04-22 23:19:16+05:30 Cprogrammer Exp mbhangui $";
#endif

int
vfstabNew(char *filesystem, long max_user, long max_size)
{
	long            quota_user, quota_size;
	char           *local_ip, *ptr, *avg_user_quota;
#ifdef sun
	struct statvfs  statbuf;
#else
	struct statfs   statbuf;
#endif

	if (max_user == -1 || max_size == -1) {
#ifdef sun
		if (statvfs(filesystem, &statbuf))
#else
		if (statfs(filesystem, &statbuf))
#endif
		{
			strerr_warn3("vfstabNew: statfs: ", filesystem, ": ", &strerr_sys);
			return (-1);
		}
		if (max_size == -1)
			quota_size = (statbuf.f_bavail * statbuf.f_bsize);
		else
			quota_size = max_size;
		if (max_user == -1) {
			getEnvConfigStr(&avg_user_quota, "AVG_USER_QUOTA", AVG_USER_QUOTA);
			quota_user = quota_size / strtoll(avg_user_quota, 0, 0);
		} else
			quota_user = max_user;
	} else {
		quota_size = max_size;
		quota_user = max_user;
	}
	if (!(local_ip = get_local_ip(PF_INET))) {
		strerr_warn1("vfstabNew: get_local_ip failed", 0);
		return (-1);
	}
	if (!(ptr = pathToFilesystem(filesystem))) {
		strerr_warn3("vfstabNew: ", filesystem, ": Not a filesystem", 0);
		return (-1);
	}
	return (vfstab_insert(ptr, local_ip, FS_ONLINE, quota_user, quota_size));
}
