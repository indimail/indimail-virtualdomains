/*
 * $Log: vget_ip_map.c,v $
 * Revision 1.1  2019-04-14 21:50:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vget_ip_map.c,v 1.1 2019-04-14 21:50:48+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("valias_insert: out of memory", 0);
	_exit(111);
}

int
vget_ip_map(char *ip, stralloc *domain)
{
	static stralloc SqlBuf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             ret = -1;

	if (!ip || !*ip)
		return (-1);
	if (!domain)
		return (-2);
	if (iopen((char *) 0) != 0)
		return (-3);
	if (!stralloc_copyb(&SqlBuf, "select high_priority domain from ip_alias_map where ipaddr = \"", 62) ||
			!stralloc_cats(&SqlBuf, ip) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("vget_ip_map: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("vget_ip_map: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-4);
	}
	if ((row = in_mysql_fetch_row(res))) {
		ret = 0;
		if (!stralloc_copys(domain, row[0]) || !stralloc_0(domain))
			die_nomem();
		domain->len--;
	}
	in_mysql_free_result(res);
	return (ret);
}
#endif
