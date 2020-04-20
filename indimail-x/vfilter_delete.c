/*
 * $Log: vfilter_delete.c,v $
 * Revision 1.1  2019-04-15 10:34:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_delete.c,v 1.1 2019-04-15 10:34:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <fmt.h>
#include <stralloc.h>
#include <strerr.h>
#endif
#include "create_table.h"
#include "iopen.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_delete: out of memory", 0);
	_exit(111);
}

int
vfilter_delete(char *emailid, int filter_no)
{
	int             err;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (iopen((char *) 0))
		return (-1);
	if (filter_no == -1) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from vfilter where emailid=\"", 48) ||
				!stralloc_cats(&SqlBuf, emailid) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from vfilter where emailid=\"", 48) ||
				!stralloc_cats(&SqlBuf, emailid) ||
				!stralloc_catb(&SqlBuf, "\" and filter_no=", 16) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, filter_no)) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return (-1);
			return (0);
		}
		strerr_warn4("vfilter_delete: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("vfilter_delete: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return ((err > 0) ? 0 : 1);
}
#endif
