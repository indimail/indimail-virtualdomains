/*
 * $Log: del_atrn_map.c,v $
 * Revision 1.1  2019-04-15 09:46:01+05:30  Cprogrammer
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
static char     sccsid[] = "$Id: del_atrn_map.c,v 1.1 2019-04-15 09:46:01+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("del_atrn_map: out of memory", 0);
	_exit(111);
}

int
del_atrn_map(char *user, char *domain, char *domain_list)
{
	static stralloc SqlBuf = {0};
	int             err;

	if (!user || !*user || !domain || !*domain || !domain_list || !*domain_list)
		return (-1);
	if (iopen((char *) 0) != 0)
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from atrn_map where pw_name = \"", 51) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\" and domain_list=\"", 19) ||
			!stralloc_cats(&SqlBuf, domain_list) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT))
				return(-1);
			return (0);
		}
		strerr_warn4("del_atrn_map: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("del_atrn_map: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return (err);
}
