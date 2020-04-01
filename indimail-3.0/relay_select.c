/*
 * $Log: relay_select.c,v $
 * Revision 1.2  2020-04-01 18:57:42+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-14 23:07:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: relay_select.c,v 1.2 2020-04-01 18:57:42+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <str.h>
#include <case.h>
#include <scan.h>
#include <getEnvConfig.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "indimail.h"

static void
die_nomem()
{
	strerr_warn1("relay_select: out of memory", 0);
	_exit(111);
}

int
relay_select(char *email, char *remoteip)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];
	char           *relay_table;
	int             len;
	long            timeout;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (iopen((char *) 0))
		return (-1);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	getEnvConfigLong(&timeout, "RELAY_CLEAR_MINUTES", RELAY_CLEAR_MINUTES);
	timeout *= 60;
	if (!stralloc_copyb(&SqlBuf, "select email FROM ", 18) ||
			!stralloc_cats(&SqlBuf, relay_table) ||
			!stralloc_catb(&SqlBuf, " WHERE ipaddr=\"", 15) ||
			!stralloc_cats(&SqlBuf, remoteip) ||
			!stralloc_catb(&SqlBuf, "\" AND timestamp>(UNIX_TIMESTAMP()-", 34) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, timeout)) ||
			!stralloc_append(&SqlBuf, ")") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
		else
			strerr_warn4("relay_select: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (0);
	} 
	res = in_mysql_store_result(&mysql[1]);
	for (len = str_len(email);(row = in_mysql_fetch_row(res));) {
		if (!case_diffb(row[0], len, email)) {
			in_mysql_free_result(res);
			return(1);
		}
	}
	in_mysql_free_result(res);
	return(0);
}
#endif
