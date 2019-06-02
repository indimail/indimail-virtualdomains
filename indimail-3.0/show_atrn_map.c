/*
 * $Log: show_atrn_map.c,v $
 * Revision 1.1  2019-04-14 23:10:12+05:30  Cprogrammer
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
#include <strerr.h>
#endif
#include "variables.h"
#include "iopen.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: show_atrn_map.c,v 1.1 2019-04-14 23:10:12+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("show_atrn_map: out of memory", 0);
	_exit(111);
}

char           *
show_atrn_map(char **user, char **domain)
{
	static stralloc SqlBuf = {0};
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!select_res) {
		if (iopen((char *) 0))
			return ((char *) 0);
		if (user && *user) {
			if (!stralloc_copyb(&SqlBuf, "select high_priority domain_list from atrn_map where pw_name=\"", 62) ||
					!stralloc_cats(&SqlBuf, *user) ||
					!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
					!stralloc_cats(&SqlBuf, *domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		} else
		if (domain && *domain) {
			if (!stralloc_copyb(&SqlBuf,
						"select high_priority pw_name,pw_domain,domain_list from atrn_map where domain_list=\"", 84) ||
					!stralloc_cats(&SqlBuf, *domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		} else
		if (!domain || !*domain || !**domain) {
			if (!stralloc_copyb(&SqlBuf, "select high_priority pw_name,pw_domain,domain_list from atrn_map", 64) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		}
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT);
				return ((char *) 0);
			}
			strerr_warn4("vchow_atrn_map: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((char *) 0);
		}
		if (!(select_res = in_mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(select_res))) {
		if (user && *user)
			return (row[0]);
		else {
			if (user)
				*user = row[0];
			if (domain)
				*domain = row[1];
			return (row[2]);
		}
	}
	in_mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
