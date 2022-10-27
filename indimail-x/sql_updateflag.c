/*
 * $Log: sql_updateflag.c,v $
 * Revision 1.3  2022-10-27 17:28:55+05:30  Cprogrammer
 * refactored sql code into do_sql()
 *
 * Revision 1.2  2019-04-20 08:18:52+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#endif
#include "iopen.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_updateflag.c,v 1.3 2022-10-27 17:28:55+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_updateflag: out of memory", 0);
	_exit(111);
}

static int
do_sql(char *user, char *domain, int flag, char *table)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (flag == -1) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, table) ||
				!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, table) ||
				!stralloc_catb(&SqlBuf, " set pw_uid = ", 14) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, flag)) ||
				!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
	}
	if (site_size == SMALL_SITE && (
				!stralloc_catb(&SqlBuf, " and pw_domain=\"", 16) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"")))
			die_nomem();
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_updateflag: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return (in_mysql_affected_rows(&mysql[1]));
}

int
sql_updateflag(char *user, char *domain, int flag)
{
	int             err;

	if (!user || !*user || !domain || !*domain)
		return (-1);
	if (iopen(0))
		return (-1);
	if ((err = do_sql(user, domain, flag, default_table)) == -1)
		return -1;
	else
	if (!err) {
		if ((err = do_sql(user, domain, flag, inactive_table)) == -1)
			return -1;
	}
	return (((err == -1 || !err) ?  1 : 0));
}
#endif
