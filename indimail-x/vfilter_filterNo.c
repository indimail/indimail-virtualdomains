/*
 * $Log: vfilter_filterNo.c,v $
 * Revision 1.1  2019-04-15 10:35:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_filterNo.c,v 1.1 2019-04-15 10:35:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
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
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_filterNo: out of memory", 0);
	_exit(111);
}

int
vfilter_filterNo(char *emailid)
{
	int             i, filter_no;
	static stralloc SqlBuf = {0};
	MYSQL_ROW       row;
	MYSQL_RES      *res;

	if (iopen((char *) 0))
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "select high_priority filter_no from vfilter where emailid=\"", 59) ||
			!stralloc_cats(&SqlBuf, emailid) ||
			!stralloc_catb(&SqlBuf, "\" and filter_no > 1 order by filter_no", 38) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return (-1);
			return (2);
		}
		strerr_warn4("vfilter_filterNo: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!(res = in_mysql_store_result(&mysql[1])))
		return (-1);
	if (!in_mysql_num_rows(res)) {
		in_mysql_free_result(res);
		return (2);
	}
	for(i = 2; (row = in_mysql_fetch_row(res)); i++) {
		scan_uint(row[0], (unsigned int *) &filter_no);
		if (i != filter_no) {
			in_mysql_free_result(res);
			return (i);
		}
	}
	in_mysql_free_result(res);
	return (i);
}
#endif
