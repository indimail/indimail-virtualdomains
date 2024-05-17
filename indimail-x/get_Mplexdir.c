/*
 * $Log: get_Mplexdir.c,v $
 * Revision 1.2  2020-04-01 18:54:53+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:18:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "indimail.h"
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "r_mkdir.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_Mplexdir.c,v 1.2 2020-04-01 18:54:53+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("get_Mplexdir: out of memory", 0);
	_exit(111);
}

char           *
get_Mplexdir(const char *username, const char *domain, int creflag, uid_t uid, gid_t gid)
{
	int             ch;
	static stralloc dirbuf = {0};
	char           *base_path;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
	};

	if (!*username)
		return ((char *) 0);
	getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	ch = tolower(*username);
	if (!stralloc_copys(&dirbuf, base_path) || !stralloc_append(&dirbuf, "/"))
		die_nomem();
	if (ch >= 'a' && ch <= 'e') {
		if (!stralloc_cats(&dirbuf, FileSystems[0]))
			die_nomem();
	} else
	if (ch >= 'f' && ch <= 'k')
	{
		if (!stralloc_cats(&dirbuf, FileSystems[1]))
			die_nomem();
	} else
	if (ch >= 'l' && ch <= 'p')
	{
		if (!stralloc_cats(&dirbuf, FileSystems[2]))
			die_nomem();
	} else
	if (ch >= 'q' && ch <= 's')
	{
		if (!stralloc_cats(&dirbuf, FileSystems[3]))
			die_nomem();
	} else {
		if (!stralloc_cats(&dirbuf, FileSystems[4]))
			die_nomem();
	}
	if (!stralloc_append(&dirbuf, "/") || !stralloc_cats(&dirbuf, domain) || !stralloc_0(&dirbuf))
		die_nomem();
	dirbuf.len--;
	if (creflag && dirbuf.len) {
		if (r_mkdir(dirbuf.s, INDIMAIL_DIR_MODE, uid, gid) == -1) {
			if (errno != EEXIST)
				strerr_warn3("getMplexdir: r_mkdir: ", dirbuf.s, ": ", &strerr_sys);
		}
	}
	return (dirbuf.s);
}
