/*
 * $Log: vfilter_select.c,v $
 * Revision 1.1  2019-04-15 10:51:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_select.c,v 1.1 2019-04-15 10:51:52+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
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
#include "create_table.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_select: out of memory", 0);
	_exit(111);
}

int
vfilter_select(char *emailid, int *filter_no, stralloc *filter_name, int *header_name, int *comparision, stralloc *keyword,
	stralloc *destination, int *bounce_action, stralloc *forward)
{
	static stralloc SqlBuf = {0};
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if (!res) {
		if (iopen((char *) 0))
			return (-1);
		if (!stralloc_copyb(&SqlBuf, "select high_priority filter_no, filter_name, header_name,", 57) ||
				!stralloc_catb(&SqlBuf, " comparision, keyword, destination, bounce_action from ", 55) ||
				!stralloc_catb(&SqlBuf, "vfilter where emailid = \"", 25) ||
				!stralloc_cats(&SqlBuf, emailid) ||
				!stralloc_catb(&SqlBuf, "\" order by filter_no", 20) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT);
				return (-2);
			} else {
				strerr_warn4("vfilter_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		}
		if (!(res = in_mysql_store_result(&mysql[1])))
			return (-1);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (filter_no)
			scan_int(row[0], filter_no);
		if (filter_name) {
			if (!stralloc_copys(filter_name, row[1]) || !stralloc_0(filter_name))
				die_nomem();
			filter_name->len--;
		}
		if (header_name) {
			scan_int(row[2], header_name);
			scan_int(row[3], comparision);
		}
		if (keyword) {
			if (!stralloc_copys(keyword, row[4]) || !stralloc_0(keyword))
				die_nomem();
			keyword->len--;
		}
		if (destination) {
			if (!stralloc_copys(destination, row[5]) || !stralloc_0(destination))
				die_nomem();
			destination->len--;
		}
		if (bounce_action)
			scan_int(row[6], bounce_action);
		if (forward) {
			if(*bounce_action == 2 || *bounce_action == 3) {
				if (!stralloc_copys(forward, row[6] + 1) || !stralloc_0(forward))
					die_nomem();
				forward->len--;
			} else
				forward->len = 0;
		}
		return (0);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return (-2);
}
#endif
