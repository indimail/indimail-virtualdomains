/*
 * $Log: sql_gethostid.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-14 23:05:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_gethostid.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#endif
#include "findhost.h"
#include "create_table.h"
#include "indimail.h"

static void
die_nomem()
{
	strerr_warn1("sql_gethostid: out of memory", 0);
	_exit(111);
}

char           *
sql_gethostid(const char *ipaddr)
{
	static stralloc hostid = {0}, SqlBuf = {0};
	const char     *ptr;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
		return ((char *) 0);
	if (!ipaddr || !*ipaddr)
		return ((char *) 0);
	if (!str_diffn(ipaddr, "localhost", 10))
		ptr = "127.0.0.1";
	else
		ptr = ipaddr;
	if (!stralloc_copyb(&SqlBuf, "select high_priority host from host_table where ipaddr=\"", 56) ||
			!stralloc_cats(&SqlBuf, ptr) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT);
		else
			strerr_warn4("sql_gethostid: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("sql_gethostid: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!(row = in_mysql_fetch_row(res))) {
		in_mysql_free_result(res);
		return ((char *) 0);
	}
	if (!stralloc_copys(&hostid, row[0]) || !stralloc_0(&hostid))
		die_nomem();
	in_mysql_free_result(res);
	return (hostid.s);
}
#endif
