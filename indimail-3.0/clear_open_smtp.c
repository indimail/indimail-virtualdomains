/*
 * $Log: clear_open_smtp.c,v $
 * Revision 1.3  2019-06-30 10:13:43+05:30  Cprogrammer
 * seperate fields in error string by commas
 *
 * Revision 1.2  2019-05-28 17:34:47+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-14 23:15:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: clear_open_smtp.c,v 1.3 2019-06-30 10:13:43+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#include "load_mysql.h"
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "iopen.h"
#include "sql_init.h"
#include "dbload.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("clear_open_smtp: out of memory", 0);
	_exit(111);
}

int
clear_open_smtp(time_t clear_seconds, int connect_all)
{
	static stralloc SqlBuf = {0};
	time_t          delete_time;
	char           *relay_table;
	char            strnum[FMT_ULONG];
	int             err;
#ifdef CLUSTERED_SITE
	DBINFO        **ptr;
	MYSQL         **mysqlptr;
#endif

	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	delete_time = time(0) - clear_seconds;
	err = 0;
#ifndef CLUSTERED_SITE
	if (iopen((char *)0))
		return (1);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
			!stralloc_cats(&SqlBuf, relay_table) ||
			!stralloc_catb(&SqlBuf, " where timestamp <= ", 20) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) delete_time)) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("clear_open_smtp: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
#else
	if (!connect_all) {
		if (iopen((char *)0))
			return (1);
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, relay_table) ||
				!stralloc_catb(&SqlBuf, " where timestamp <= ", 20) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) delete_time)) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
				return (0);
			}
			strerr_warn4("clear_open_smtp: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		}
		return (0);
	}
	if (OpenDatabases())
		return (1);
	for (mysqlptr = MdaMysql, ptr = RelayHosts;(*ptr);mysqlptr++, ptr++) {
		if ((*ptr)->fd == -1) {
			strnum[fmt_uint(strnum, (*ptr)->port)] = 0;
			strerr_warn16("in_mysql_real_connect: ", (*ptr)->database, "@", (*ptr)->server,
					", domain ", (*ptr)->domain, ", mdahost ", (*ptr)->mdahost, ", user ",
					(*ptr)->user, ", port ", strnum, ", socket ",
					(*ptr)->socket ? (*ptr)->socket : "TCP/IP", ": ", (char *) in_mysql_error(*mysqlptr), 0);
			continue;
		}
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, relay_table) ||
				!stralloc_catb(&SqlBuf, " where timestamp <= ", 20) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) delete_time)) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query((*mysqlptr), SqlBuf.s)) {
			if (in_mysql_errno((*mysqlptr)) == ER_NO_SUCH_TABLE) {
				sql_init(1, *mysqlptr);
				create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
				in_mysql_close(*mysqlptr);
				is_open = 0;
				continue;
			}
			strnum[fmt_uint(strnum, (*ptr)->port)] = 0;
			strerr_warn16("in_mysql_real_connect: ", (*ptr)->database, "@", (*ptr)->server,
					", domain ", (*ptr)->domain, ", mdahost ", (*ptr)->mdahost, ", user ",
					(*ptr)->user, ", port ", strnum, ", socket ",
					(*ptr)->socket ? (*ptr)->socket : "TCP/IP", ": ", (char *) in_mysql_error(*mysqlptr), 0);
			err = 1;
			continue;
		}
	}
#endif
	return (err);
}
#endif
