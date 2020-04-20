/*
 * $Log: sql_getip.c,v $
 * Revision 1.1  2019-04-10 10:08:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include <mysql.h>
#include <mysqld_error.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "create_table.h"
#include "findhost.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_getip.c,v 1.1 2019-04-10 10:08:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

static void
die_nomem()
{
	strerr_warn1("sql_getip: out of memory", 0);
	_exit(111);
}

char           *
sql_getip(char *hostid)
{
	static stralloc ipaddr = {0}, SqlBuf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
		return ((char *) 0);
	if (!hostid || !*hostid)
		return ((char *) 0);
	if (!stralloc_copyb(&SqlBuf, "select high_priority ipaddr from host_table where  host=\"", 57) ||
		!stralloc_cats(&SqlBuf, hostid) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT);
		else
			strerr_warn4("sql_getip: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("sql_getip: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!(row = in_mysql_fetch_row(res))) {
		in_mysql_free_result(res);
		return ((char *) 0);
	}
	if (!stralloc_copys(&ipaddr, row[0]) || !stralloc_0(&ipaddr))
		die_nomem();
	in_mysql_free_result(res);
	return (ipaddr.s);
}
#endif
