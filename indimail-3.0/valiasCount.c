/*
 * $Log: valiasCount.c,v $
 * Revision 1.1  2019-04-15 11:59:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: valiasCount.c,v 1.1 2019-04-15 11:59:09+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "get_real_domain.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("valiasCount: out of memory", 0);
	_exit(111);
}

long
valiasCount(char *alias, char *domain)
{
	static stralloc SqlBuf = {0};
	char           *real_domain;
	unsigned long   row_count;
	MYSQL_ROW       row;
	MYSQL_RES      *select_res;

	if (!domain || !*domain)
		return(-1);
	if (iopen((char *) 0))
		return(-1);
	if (!(real_domain = get_real_domain(domain)))
		real_domain = domain;
	if (!stralloc_copyb(&SqlBuf, "select count(*) from valias where alias=\"", 41) ||
			!stralloc_cats(&SqlBuf, alias) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("valiasCount: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return(-1);
	}
	if (!(select_res = in_mysql_store_result(&mysql[1])))
		die_nomem();
	if ((row = in_mysql_fetch_row(select_res))) {
		scan_ulong(row[0], &row_count);
		in_mysql_free_result(select_res);
		return (row_count);
	}
	in_mysql_free_result(select_res);
	return(0);
}
#endif
