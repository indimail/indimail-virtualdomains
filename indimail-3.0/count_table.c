/*
 * $Log: count_table.c,v $
 * Revision 1.2  2019-05-02 14:36:41+05:30  Cprogrammer
 * added argument to specify a 'where clause'
 *
 * Revision 1.1  2019-04-15 09:34:07+05:30  Cprogrammer
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
#include <scan.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#include "iopen.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: count_table.c,v 1.2 2019-05-02 14:36:41+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("count_table: out of memory", 0);
	_exit(111);
}

long
count_table(char *table, char *condition)
{
	static stralloc SqlBuf = {0};
	long            row_count;
	MYSQL_ROW       row;
	MYSQL_RES      *select_res;

	if (iopen((char *) 0))
		return (-1);
	if (!table || !*table)
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "select count(*) from ", 21) ||
			!stralloc_cats(&SqlBuf, table))
		die_nomem();
	if (condition) {
		if (!stralloc_append(&SqlBuf, " ") ||
				!stralloc_cats(&SqlBuf, condition))
			die_nomem();
	}
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			return (0);
		strerr_warn4("count_table: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (!(select_res = in_mysql_store_result(&mysql[1])))
		return (-1);
	if ((row = in_mysql_fetch_row(select_res))) {
		scan_ulong(row[0], (unsigned long *) &row_count);
		in_mysql_free_result(select_res);
		return (row_count);
	}
	/*- should not happen */
	in_mysql_free_result(select_res);
	return (-1);
}
