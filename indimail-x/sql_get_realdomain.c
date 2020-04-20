/*
 * $Log: sql_get_realdomain.c,v $
 * Revision 1.1  2019-04-18 15:46:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_get_realdomain.c,v 1.1 2019-04-18 15:46:35+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include <strerr.h>
#endif
#include "create_table.h"
#include "findhost.h"
#include "indimail.h"

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static void
die_nomem()
{
	strerr_warn1("sql_get_realdomain: out of memory", 0);
	_exit(111);
}

char *
sql_get_realdomain(char *aliasdomain)
{
	static stralloc prevDomainVal = {0}, SqlBuf = {0}, buf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;

#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE") && prevDomainVal.len && 
		!str_diffn(prevDomainVal.s, aliasdomain, prevDomainVal.len + 1)) {
		if (buf.len)
			return (buf.s);
		else
			return ((char *) 0);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	if (open_central_db((char *) 0))
		return ((char *) 0);
	if (!stralloc_catb(&SqlBuf, "select high_priority domain from aliasdomain where alias=\"", 58) ||
		!stralloc_cats(&SqlBuf, aliasdomain) ||
		!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT);
			return ((char *) 0);
		}
		strerr_warn4("sql_get_realdomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("sql_get_realdomain: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (in_mysql_num_rows(res) == 0) {
		in_mysql_free_result(res);
		if (!stralloc_copys(&prevDomainVal, aliasdomain) || !stralloc_0(&prevDomainVal))
			die_nomem();
		prevDomainVal.len--;
		return ((char *) 0);
	}
	row = in_mysql_fetch_row(res);
	if (!stralloc_copys(&buf, row[0]) || !stralloc_0(&buf))
		die_nomem();
	buf.len--;
	in_mysql_free_result(res);
	if (!stralloc_copys(&prevDomainVal, aliasdomain) || !stralloc_0(&prevDomainVal))
		die_nomem();
	prevDomainVal.len--;
	return (buf.s);
}

#ifdef QUERY_CACHE
void
sql_get_realdomain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif
