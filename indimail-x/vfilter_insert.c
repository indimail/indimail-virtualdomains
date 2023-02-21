/*
 * $Log: vfilter_insert.c,v $
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-15 10:47:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_insert.c,v 1.2 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
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
#include <fmt.h>
#include <subfd.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "vfilter_filterNo.h"
#include "create_table.h"
#include "variables.h"
#include "common.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_insert: out of memory", 0);
	_exit(111);
}

int
vfilter_insert(char *emailid, char *filter_name, int header_name, int comparision, char *keyword, char *folder, int bounce_action,
	char *faddr)
{
	int             err, filter_no;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (iopen((char *) 0))
		return (-1);
	if(comparision == 5 || comparision == 6)
		filter_no = comparision - 5;
	else
	if((filter_no = vfilter_filterNo(emailid)) == -1) {
		strerr_warn1("vfilter_insert: failed to obtain filter No", 0);
		return(-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into vfilter ", 33) ||
			!stralloc_catb(&SqlBuf, "(emailid, filter_no, filter_name, header_name, comparision, keyword,", 68) ||
			!stralloc_catb(&SqlBuf, " destination, bounce_action) values (\"", 38) ||
			!stralloc_cats(&SqlBuf, emailid) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf,  strnum, fmt_uint(strnum, (unsigned int) filter_no)) ||
			!stralloc_catb(&SqlBuf, ", \"", 3) ||
			!stralloc_cats(&SqlBuf, filter_name) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf,  strnum, fmt_uint(strnum, (unsigned int) header_name)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf,  strnum, fmt_uint(strnum, (unsigned int) comparision)) ||
			!stralloc_catb(&SqlBuf, ", \"", 3) ||
			!stralloc_cats(&SqlBuf, keyword) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, folder) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_catb(&SqlBuf,  strnum, fmt_uint(strnum, (unsigned int) bounce_action)))
		die_nomem();
	if ((bounce_action == 2 || bounce_action == 3) && !stralloc_cats(&SqlBuf, faddr))
		die_nomem();
	if (!stralloc_catb(&SqlBuf, "\")", 2) || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return(-1);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("vfilter_insert: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		} else {
			strerr_warn4("vfilter_insert: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("vfilter_insert: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return(-1);
	}
	if(!verbose)
		return (err > 0 ? 0 : 1);
	if(err) {
		subprintfe(subfdout, "vfilter_insert", "added filter No %d for %s\n", filter_no, emailid);
		flush("vfilter_insert");
	} else {
		subprintfe(subfderr, "vfilter_insert", "vcfilter: filter No %d failed for %s\n", filter_no, emailid);
		errflush("vfilter_insert");
	}
	return (err > 0 ? 0 : 1);
}
#endif
