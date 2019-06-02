/*
 * $Log: sql_setquota.c,v $
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
static char     sccsid[] = "$Id: sql_setquota.c,v 1.1 2019-04-14 22:49:14+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_setquota: out of memory", 0);
	_exit(111);
}

int
sql_setquota(char *user, char *domain, char *quota)
{
	char           *tmpstr;
	unsigned long   q1, q2;
	int             i, no_of_rows, err;
	char            strnum[FMT_ULONG];
	static stralloc SqlBuf = {0}, tmpQuota = {0};
	struct passwd  *pw;

	if (iopen((char *) 0))
		return (-1);
	if (!(pw = sql_getpw(user, domain))) {
		strerr_warn4("sql_setquota: ", user, "@", domain, 0);
		return (0);
	}
	if (!str_diff(pw->pw_shell, quota))
		return (1);
	if ((*quota == '+') || (*quota == '-')) {
		scan_ulong(pw->pw_shell, &q1);
		scan_ulong(quota, &q2);
		strnum[i = fmt_ulong(strnum, q1 + q2)] = 0;
		if (!stralloc_copyb(&tmpQuota, strnum, i) || !stralloc_0(&tmpQuota))
			die_nomem();
		tmpQuota.len--;
	} else {
		if (!stralloc_copys(&tmpQuota, quota) || !stralloc_0(&tmpQuota))
			die_nomem();
		tmpQuota.len--;
	}
	if (site_size == LARGE_SITE) {
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, tmpstr) ||
				!stralloc_catb(&SqlBuf, " set pw_shell = \"", 17) ||
				!stralloc_cat(&SqlBuf, &tmpQuota) ||
				!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, default_table) ||
				!stralloc_catb(&SqlBuf, " set pw_shell = \"", 17) ||
				!stralloc_cat(&SqlBuf, &tmpQuota) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_setquota: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	no_of_rows = in_mysql_affected_rows(&mysql[1]);
	if (!no_of_rows && site_size == SMALL_SITE) {
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " set pw_shell = \"", 17) ||
				!stralloc_cat(&SqlBuf, &tmpQuota) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
		err = 0;
		if (mysql_query(&mysql[1], SqlBuf.s) && (err = in_mysql_errno(&mysql[1])) != ER_NO_SUCH_TABLE) {
			strerr_warn4("sql_setquota: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
		if (err == ER_NO_SUCH_TABLE)
			no_of_rows = 0;
		else
			no_of_rows = in_mysql_affected_rows(&mysql[1]);
	}
	if (no_of_rows == -1) {
		strerr_warn2("sql_setquota: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
#ifdef QUERY_CACHE
	else
	if (no_of_rows == 1)
		sql_getpw_cache(0);
#endif
	return (no_of_rows);
}
