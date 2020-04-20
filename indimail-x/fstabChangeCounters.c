/*
 * $Log: fstabChangeCounters.c,v $
 * Revision 1.1  2019-04-15 10:06:23+05:30  Cprogrammer
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
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#endif
#include "iopen.h"
#include "get_local_ip.h"
#include "open_master.h"
#include "variables.h"
#include "pathToFilesystem.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: fstabChangeCounters.c,v 1.1 2019-04-15 10:06:23+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("fstabChangeCounters: out of memory", 0);
	_exit(111);
}

int
fstabChangeCounter(char *filesystem, char *mdahost, long user_count, long size_count)
{
	int             err, idx;
	char           *ptr, *mhost;
	char            strnum[FMT_ULONG];
	static stralloc SqlBuf = {0};

#ifdef CLUSTERED_SITE
	idx = 0;
	if (!mdahost) {
		if (iopen((char *) 0))
			return (-1);
		mysql[0] = mysql[1];
		mysql[0].affected_rows= ~(my_ulonglong) 0;
		isopen_cntrl = 2; /*- same connection as from host.mysql */
		if (!(mhost = get_local_ip(PF_INET))) {
			strerr_warn1("fstabChangeCounters: get_local_ip: ", &strerr_sys);
			return(-1);
		}
	} else {
		mhost = mdahost;
		if (open_master()) {
			strerr_warn1("fstabChangeCounter: failed to open master db", 0);
			return (-1);
		}
	}
#else
	idx = 1;
	if (iopen((char *) 0)) {
		strerr_warn1("fstabChangeCounter: failed to open local db", 0);
		return (-1);
	}
#endif
	if (!(ptr = pathToFilesystem(filesystem))) {
		strerr_warn3("fstabChangeCounter: pathToFilesystem: ", filesystem, ": ", &strerr_sys);
		return (-1);
	}
	if (user_count && size_count) {
		if (!stralloc_copyb(&SqlBuf, "update fstab set cur_users=cur_users+", 37) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, user_count)) ||
				!stralloc_catb(&SqlBuf, ", cur_size=cur_size+", 20) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, size_count)) ||
				!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, ptr) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else
	if (user_count) {
		if (!stralloc_copyb(&SqlBuf, "update fstab set cur_users=cur_users+", 37) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, user_count)) ||
				!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, ptr) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else
	if (size_count) {
		if (!stralloc_copyb(&SqlBuf, "update fstab set cur_size=cur_size+", 35) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, size_count)) ||
				!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, ptr) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[idx], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[idx]) == ER_NO_SUCH_TABLE) {
			create_table(mdahost ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
			if (!mysql_query(&mysql[idx], SqlBuf.s))
				return (0);
		}
		strerr_warn4("fstabChangeCounter: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[idx]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[idx])) == -1) {
		strerr_warn2("fstabChangeCounter: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
