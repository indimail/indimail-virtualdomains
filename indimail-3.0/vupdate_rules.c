/*
 * $Log: vupdate_rules.c,v $
 * Revision 1.2  2020-04-01 18:59:31+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-17 09:43:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vupdate_rules.c,v 1.2 2020-04-01 18:59:31+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("vupdate_rules: out of memory", 0);
	_exit(111);
}

int
vupdate_rules(int fdm)
{
	static stralloc tmpbuf = {0};
	MYSQL_ROW       row;
	MYSQL_RES      *res;
	char           *relay_table;

	if (iopen((char *)0))
		return (1);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	if (!stralloc_copyb(&tmpbuf, "select high_priority ipaddr from ", 33) ||
			!stralloc_cats(&tmpbuf, relay_table) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (mysql_query(&mysql[1], tmpbuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
				return (1);
			if (mysql_query(&mysql[1], tmpbuf.s)) {
				strerr_warn4("vupdate_rules: mysql_query [", tmpbuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				return (1);
			}
		} else {
			strerr_warn4("vupdate_rules: mysql_query [", tmpbuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		}
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("vupdate_rules: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
	while ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(&tmpbuf, row[0]) ||
				!stralloc_catb(&tmpbuf, ":allow,RELAYCLIENT=\"\"\n", 22))
			die_nomem();
		if (write(fdm, tmpbuf.s, tmpbuf.len) == -1) {
			strerr_warn1("vupdate_rules: write: ", &strerr_sys);
			in_mysql_free_result(res);
			return (1);
		}
	}
	in_mysql_free_result(res);
	return (0);
}
#endif
