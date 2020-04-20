/*
 * $Log: CreateDomainDirs.c,v $
 * Revision 1.2  2020-04-01 18:53:59+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-14 18:31:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"
#include "r_mkdir.h"

#ifndef	lint
static char     sccsid[] = "$Id: CreateDomainDirs.c,v 1.2 2020-04-01 18:53:59+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("CreateDomainDirs: out of memory", 0);
	_exit(111);
}

int
CreateDomainDirs(char *domain, uid_t uid, gid_t gid)
{
	int             i, len, dlen;
	static stralloc dirbuf = {0};
	char           *base_path;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
		"",
	};

	if (!domain || !*domain)
		return (1);
	getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	if (!stralloc_copys(&dirbuf, base_path) ||
			!stralloc_append(&dirbuf, "/"))
		die_nomem();
	len = dirbuf.len;
	dlen = str_len(domain);
	for(i = 0;*FileSystems[i];i++) {
		if (!stralloc_catb(&dirbuf, FileSystems[i], i < 4 ? 3 : 6) ||
				!stralloc_append(&dirbuf, "/") ||
				!stralloc_catb(&dirbuf, domain, dlen) ||
				!stralloc_0(&dirbuf))
			die_nomem();
		if (r_mkdir(dirbuf.s, INDIMAIL_DIR_MODE, uid, gid)) {
			if (errno != EEXIST)
				strerr_warn3("CreateDomainDirs: r_mkdir: ", dirbuf.s, ": ", &strerr_sys);
		}
		dirbuf.len = len;
	}
	return (0);
}
