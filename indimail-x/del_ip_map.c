/*
 * $Log: del_ip_map.c,v $
 * Revision 1.1  2019-04-15 09:51:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: del_ip_map.c,v 1.1 2019-04-15 09:51:23+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("del_ip_map: out of memory", 0);
	_exit(111);
}

int
del_ip_map(char *ip, char *domain)
{
	static stralloc SqlBuf = {0};
	int             err;

	if (!ip || !*ip || !domain || !*domain)
		return (-1);
	if (iopen((char *) 0) != 0)
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from ip_alias_map where ipaddr=\"", 52) ||
			!stralloc_cats(&SqlBuf, ip) ||
			!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT))
				return(-1);
			return(0);
		}
		strerr_warn4("del_ip_map: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("del_ip_map: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return (err);
}
#endif
