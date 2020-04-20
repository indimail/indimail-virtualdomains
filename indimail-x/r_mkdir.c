/*
 * $Log: r_mkdir.c,v $
 * Revision 1.1  2019-04-14 18:32:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stralloc.h>
#include <strerr.h>

static stralloc _dirbuf = { 0 };

int
r_mkdir(char *dir, mode_t mode, uid_t uid, gid_t gid)
{
	char           *ptr;
	int             i;

	if (!stralloc_copys(&_dirbuf, dir) || !stralloc_0(&_dirbuf))
		strerr_die1sys(111, "r_mkdir: out of memory: ");
	for (ptr = _dirbuf.s + 1; *ptr; ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(_dirbuf.s, F_OK)) {
				if ((i = mkdir(_dirbuf.s, mode)) == -1 || (i = chown(_dirbuf.s, uid, gid)) == -1)
					return (i);
			}
			*ptr = '/';
		}
	}
	if (access(_dirbuf.s, F_OK)) {
		if ((i = mkdir(_dirbuf.s, mode)) == -1 || (i = chown(_dirbuf.s, uid, gid)) == -1)
			return (i);
	}
	return (0);
}
