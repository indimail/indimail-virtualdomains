/*
 * $Log: show_ip_map.c,v $
 * Revision 1.1  2019-04-15 09:51:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: show_ip_map.c,v 1.1 2019-04-15 09:51:18+05:30 Cprogrammer Exp mbhangui $";
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
	strerr_warn1("show_ip_map: out of memory", 0);
	_exit(111);
}

int
show_ip_map(int first, stralloc *ip, stralloc *domain, char *domain_filter)
{
	static int      more = 0;
	static MYSQL_RES *res;
	MYSQL_ROW       row;
	static stralloc SqlBuf = {0};

	if (first == 1) {
		if (iopen((char *) 0) != 0)
			return (-1);
		if(domain_filter && *domain_filter) {
			if (!stralloc_copyb(&SqlBuf, "select high_priority ipaddr, domain from ip_alias_map where domain=\"", 68) ||
					!stralloc_cats(&SqlBuf, domain_filter) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		} else {
			if (!stralloc_copyb(&SqlBuf, "select high_priority ipaddr, domain from ip_alias_map", 53) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		}
		if (res)
			in_mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
				create_table(ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT);
			else {
				strerr_warn4("show_ip_map: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
			return (0);
		}
		if (!(res = in_mysql_store_result(&mysql[1]))) {
			strerr_warn2("show_ip_map: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (0);
		}
	} else
	if (more == 0)
		return (0);
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copyb(ip, row[0], 18) || !stralloc_0(ip))
			die_nomem();
		if (!stralloc_copys(domain, row[1]) || !stralloc_0(domain))
			die_nomem();
		more = 1;
		return (1);
	}
	more = 0;
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return (0);
}
#endif
