/*
 * $Log: vfilter_update.c,v $
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-15 11:15:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_update.c,v 1.2 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
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
#include "create_table.h"
#include "common.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_update: out of memory", 0);
	_exit(111);
}

int
vfilter_update(char *emailid, int filter_no, int header_name, int comparision,
		char *keyword, char *folder, int bounce_action, char *faddr)
{
	int             err, terr;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (iopen((char *) 0))
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "update low_priority vfilter set header_name=", 44) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) header_name)) ||
			!stralloc_catb(&SqlBuf, ", comparision=", 14) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) comparision)) ||
			!stralloc_catb(&SqlBuf, ", keyword=\"", 11) ||
			!stralloc_cats(&SqlBuf, keyword) ||
			!stralloc_catb(&SqlBuf, "\", destination=\"", 16) ||
			!stralloc_cats(&SqlBuf, folder) ||
			!stralloc_catb(&SqlBuf, "\", bounce_action=\"", 18) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) bounce_action)))
		die_nomem();
	if ((bounce_action == 2 || bounce_action == 3) && !stralloc_cats(&SqlBuf, faddr))
		die_nomem();
	if (!stralloc_catb(&SqlBuf, "\" where emailid=\"", 17) ||
			!stralloc_cats(&SqlBuf, emailid) ||
			!stralloc_catb(&SqlBuf, "\" and filter_no=", 16) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) filter_no)) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT);
			return (1);
		}
		strerr_warn4("vfilter_update: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("vfilter_update: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	terr = 0;
	if(!verbose)
		return ((err >= 0 && !terr) ? 0 : 1);
	if(err) {
		subprintfe(subfdout, "vfilter_update",
				"updated filter no %d header %d keyword [%s] comparision %d folder %s bounce_action %d email [%s]\n",
				filter_no, header_name, keyword, comparision, folder, bounce_action, emailid);
		flush("vfilter_update");
	} else {
		strnum[fmt_uint(strnum, (unsigned int) filter_no)] = 0;
		strerr_warn5("vfilter_update: No filter No ", strnum, " for ", emailid, " or no filter to update", 0);
	}
	return ((err >= 0 && !terr) ? 0 : 1);
}
#endif
