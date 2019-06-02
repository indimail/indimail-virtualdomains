/*
 * $Log: vhostid_select.c,v $
 * Revision 1.1  2019-04-11 08:57:31+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#endif
#include "findhost.h"
#include "create_table.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_select.c,v 1.1 2019-04-11 08:57:31+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysql.h>
#include <mysqld_error.h>

static void
die_nomem()
{
	strerr_warn1("vhostid_select: out of memory", 0);
	_exit(111);
}

char           *
vhostid_select()
{
	static stralloc SqlBuf = {0}, resbuf = {0};
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
		return ((char *) 0);
	if (!res) {
		if (!stralloc_copyb(&SqlBuf, "select high_priority host, ipaddr from host_table", 49) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
				if (create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
					return ((char *) 0);
				return ((char *) 0);
			} else {
				strerr_warn4("vhostid_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
				return ((char *) 0);
			}
		}
		if (!(res = in_mysql_store_result(&mysql[0])))
			return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(&resbuf, row[0]) ||
				!stralloc_append(&resbuf, " ") ||
				!stralloc_cats(&resbuf, row[1]) || !stralloc_0(&resbuf))
			die_nomem();
		return (resbuf.s);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif
