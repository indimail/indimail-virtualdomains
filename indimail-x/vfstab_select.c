/*
 * $Log: vfstab_select.c,v $
 * Revision 1.1  2019-04-15 13:03:37+05:30  Cprogrammer
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
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "variables.h"
#include "findhost.h"
#include "iopen.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_select.c,v 1.1 2019-04-15 13:03:37+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vfstab_select: out of memory", 0);
	_exit(111);
}

char           *
vfstab_select(stralloc *host, int *status, long *max_users, long *cur_users, long *max_size, long *cur_size)
{
	static stralloc SqlBuf = {0}, FileSystem = {0};
	static MYSQL_RES *res;
	MYSQL_ROW       row;
	int             i;

	if (!res) {
#ifdef CLUSTERED_SITE
		i = 0;
		if (open_central_db(0))
			return ((char *) 0);
#else
		i = 1;
		if (iopen(0))
			return ((char *) 0);
#endif
		if (!stralloc_copyb(&SqlBuf, "select filesystem,host,status,max_users,cur_users,max_size,cur_size from fstab", 78))
			die_nomem();
		if (host && host->len) {
			if (!stralloc_catb(&SqlBuf, " where host=\"", 13) ||
					!stralloc_cat(&SqlBuf, host) ||
					!stralloc_append(&SqlBuf, "\""))
				die_nomem();
		}
		if (!stralloc_catb(&SqlBuf, " order by filesystem", 20) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[i], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
				if (create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT))
					return ((char *) 0);
				return ((char *) 0);
			} else {
				strerr_warn4("vfstab_select: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
				return ((char *) 0);
			}
		}
		if (!(res = in_mysql_store_result(&mysql[i])))
			return ((char *) 0);
	} 
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(&FileSystem, row[0]) ||
				!stralloc_0(&FileSystem))
			die_nomem();
		if (host) {
			if (!stralloc_copys(host, row[1]) ||
					!stralloc_0(host))
				die_nomem();
			host->len--;
		}
		if (status)
			scan_int(row[2], status);
		if (max_users)
			scan_long(row[3], max_users);
		if (cur_users)
			scan_long(row[4], cur_users);
		if (max_size)
			scan_long(row[5], max_size);
		if (cur_size)
			scan_long(row[6], cur_size);
		return (FileSystem.s);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
