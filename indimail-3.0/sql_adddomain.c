/*
 * $Log: sql_adddomain.c,v $
 * Revision 1.1  2019-04-20 08:34:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "munch_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_adddomain.c,v 1.1 2019-04-20 08:34:29+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_adddomain: out of memory", 0);
	_exit(111);
}

int
sql_adddomain(char *domain)
{
	char           *tmpstr;
	static stralloc SqlBuf = {0};
	int             err;

	if ((err = iopen((char *) 0)))
		return (err);
	if (site_size == LARGE_SITE) {
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
		if (!stralloc_copyb(&SqlBuf, "CREATE TABLE ", 13) ||
				!stralloc_cats(&SqlBuf, tmpstr) ||
				!stralloc_catb(&SqlBuf, " ( ", 3) ||
				!stralloc_cats(&SqlBuf, LARGE_TABLE_LAYOUT) ||
				!stralloc_catb(&SqlBuf, " )", 2) || !stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "CREATE TABLE IF NOT EXISTS ", 27) ||
				!stralloc_cats(&SqlBuf, default_table) ||
				!stralloc_catb(&SqlBuf, " ( ", 3) ||
				!stralloc_cats(&SqlBuf, SMALL_TABLE_LAYOUT) ||
				!stralloc_catb(&SqlBuf, " )", 2) || !stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_adddomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
	if (site_size == SMALL_SITE) {
		if (!stralloc_copyb(&SqlBuf, "CREATE TABLE IF NOT EXISTS ", 27) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " ( ", 3) ||
				!stralloc_cats(&SqlBuf, SMALL_TABLE_LAYOUT) ||
				!stralloc_catb(&SqlBuf, " )", 2) || !stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_adddomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		}
	}
	return (0);
}
