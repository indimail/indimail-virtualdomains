/*
 * $Log: hostcntrl_select.c,v $
 * Revision 1.2  2019-04-22 23:11:03+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.1  2019-04-14 22:50:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: hostcntrl_select.c,v 1.2 2019-04-22 23:11:03+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "variables.h"
#include "create_table.h"
#include "findhost.h"

static void
die_nomem()
{
	strerr_warn1("hostcntrl_select: out of memory", 0);
	_exit(111);
}

int
hostcntrl_select(char *user, char *domain, time_t *tmval, stralloc *hostid)
{
	static stralloc SqlBuf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if(open_central_db(0))
		return(1);
	if (!stralloc_copyb(&SqlBuf, "select high_priority host,unix_timestamp(timestamp) from ", 57) ||
			!stralloc_cats(&SqlBuf, cntrl_table) ||
			!stralloc_catb(&SqlBuf, " where  pw_name=\"", 17) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("hostcntrl_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	}
	if(!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("hostcntrl_select: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(hostid, row[0]) || !stralloc_0(hostid))
			die_nomem();
		hostid->len--;
		if (tmval)
			scan_ulong(row[1], (unsigned long *) tmval);
		in_mysql_free_result(res);
		return (0);
	}
	in_mysql_free_result(res);
	return (1);
}

MYSQL_ROW
hostcntrl_select_all()
{
	static stralloc SqlBuf = {0};
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if(!select_res) {
		if(open_central_db(0))
			return(0);
		if (!stralloc_copyb(&SqlBuf, "select high_priority pw_name, pw_domain, host, unix_timestamp(timestamp) from ", 78) ||
				!stralloc_cats(&SqlBuf, cntrl_table) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if(in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
				create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
				return (0);
			} 
			strerr_warn4("hostcntrl_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
			return (0);
		}
		if(!(select_res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("hostcntrl_select: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (0);
		}
	}
	if ((row = in_mysql_fetch_row(select_res)))
		return (row);
	in_mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return (0);
}
#endif
