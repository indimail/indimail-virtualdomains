/*
 * $Log: sql_setquota.c,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2022-10-27 17:21:57+05:30  Cprogrammer
 * refactored sql code into do_sql()
 *
 * Revision 1.1  2019-04-14 22:49:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <str.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "munch_domain.h"
#include "sql_getpw.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_setquota.c,v 1.3 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_setquota: out of memory", 0);
	_exit(111);
}

static int
do_sql(const char *user, const char *domain, const char *quota, const char *table)
{
	static stralloc SqlBuf = {0};

	if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
			!stralloc_cats(&SqlBuf, table) ||
			!stralloc_catb(&SqlBuf, " set pw_shell = \"", 17) ||
			!stralloc_cats(&SqlBuf, quota) ||
			!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "\""))
		die_nomem();
	if (site_size == SMALL_SITE &&
			(!stralloc_catb(&SqlBuf, " and pw_domain = \"", 18) ||
			 !stralloc_cats(&SqlBuf, domain) ||
			 !stralloc_append(&SqlBuf, "\"")))
		die_nomem();
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_setquota: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return (in_mysql_affected_rows(&mysql[1]));
}

int
sql_setquota(const char *user, const char *domain, const char *quota)
{
	char           *tmpstr;
	unsigned long   q1, q2;
	int             i, no_of_rows;
	char            strnum[FMT_ULONG];
	struct passwd  *pw;

	if (iopen((char *) 0))
		return (-1);
	if (!(pw = sql_getpw(user, domain))) {
		strerr_warn4("sql_setquota: ", user, "@", domain, 0);
		return (0);
	}
	if (!str_diff(pw->pw_shell, quota))
		return (1);
	if (site_size == LARGE_SITE) {
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
	} else
		tmpstr = default_table;
	if ((*quota == '+') || (*quota == '-')) {
		scan_ulong(pw->pw_shell, &q1);
		scan_ulong(quota, &q2);
		strnum[i = fmt_ulong(strnum, q1 + q2)] = 0;
	}
	if ((no_of_rows = do_sql(user, domain, (*quota == '+' || *quota == '-') ? strnum : quota, tmpstr)) == -1)
		return -1;
	else
	if (site_size == SMALL_SITE && !no_of_rows &&
			(no_of_rows = do_sql(user, domain, (*quota == '+' || *quota == '-') ? strnum : quota, inactive_table)) == -1)
		return -1;
#ifdef QUERY_CACHE
	else
	if (no_of_rows == 1)
		sql_getpw_cache(0);
#endif
	return (no_of_rows);
}
