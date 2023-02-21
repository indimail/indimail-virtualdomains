/*
 * $Log: vquota_select.c,v $
 * Revision 1.1  2019-04-15 12:32:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vquota_select.c,v 1.1 2019-04-15 12:32:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#ifdef HAVE_UNISTD_H
#define XOPEN_SOURCE = 600
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#endif
#include "variables.h"
#include "iopen.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("vquota_select: out of memory", 0);
	_exit(111);
}

int
vquota_select(stralloc *user, stralloc *domain, mdir_t *quota, time_t *timestamp)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];
	static MYSQL_RES *quota_res;
	time_t          tmp;
	MYSQL_ROW       row;

	tmp = (timestamp && *timestamp) ? *timestamp : time(0);
	if (!quota_res) {
		if (iopen((char *) 0))
			return (-1);
		if (domain->len) {
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority user, domain, quota, UNIX_TIMESTAMP(timestamp) "
					"from userquota where quota != 0 and UNIX_TIMESTAMP(timestamp) < ", 132) ||
					!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) tmp)) ||
					!stralloc_catb(&SqlBuf, " and domain=\"", 13) ||
					!stralloc_cat(&SqlBuf, domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		} else {
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority user, domain, quota, UNIX_TIMESTAMP(timestamp) "
					"from userquota where quota != 0 and UNIX_TIMESTAMP(timestamp) < ", 132) ||
					!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) tmp)) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		}
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
					return (-1);
				return (0);
			} else {
				strerr_warn4("vquota_select: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		}
		if (!(quota_res = in_mysql_store_result(&mysql[1])))
			return (0);
	}
	if ((row = in_mysql_fetch_row(quota_res))) {
		if (!stralloc_copys(user, row[0]) || !stralloc_0(user))
			die_nomem();
		user->len--;
		if (domain) {
			if (!stralloc_copys(domain, row[1]) || !stralloc_0(domain))
				die_nomem();
			domain->len--;
		}
		if (quota)
			scan_ulong(row[2], (unsigned long *) quota);
		if (timestamp)
			scan_ulong(row[3], (unsigned long *) timestamp);
		return (1);
	}
	in_mysql_free_result(quota_res);
	quota_res = (MYSQL_RES *) 0;
	return (0);
}
#endif
