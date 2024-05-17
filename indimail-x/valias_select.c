/*
 * $Log: valias_select.c,v $
 * Revision 1.1  2019-04-22 23:21:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: valias_select.c,v 1.1 2019-04-22 23:21:55+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
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
#include "get_real_domain.h"
#include "create_table.h"
#include "variables.h"

static stralloc SqlBuf = {0};

static void
die_nomem()
{
	strerr_warn1("valias_select: out of memory", 0);
	_exit(111);
}

char           *
valias_select(const char *alias, const char *domain)
{
	const char     *real_domain;
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!domain || !*domain || !alias || !*alias)
		return((char *) 0);
	if (!select_res) {
		if (iopen((char *) 0))
			return ((char *) 0);
		if (!(real_domain = get_real_domain(domain)))
			real_domain = domain;
		if (!stralloc_copyb(&SqlBuf, "select high_priority valias_line from valias where alias=\"", 58) ||
				!stralloc_cats(&SqlBuf, alias) ||
				!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return ((char *) 0);
			}
			strerr_warn4("valias_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((char *) 0);
		}
		if (!(select_res = in_mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(select_res)))
		return (row[0]);
	in_mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}

int
valias_track(const char *valias_line, stralloc *alias, stralloc *domain)
{
	int             err;
	MYSQL_ROW       row;
	static MYSQL_RES *res;

	if (!res) {
		if ((err = iopen((char *) 0)) != 0)
			return (-1);
		if (!stralloc_copyb(&SqlBuf, "select high_priority alias, domain from valias where valias_line = \"", 68) ||
				!stralloc_cats(&SqlBuf, valias_line) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return (-1);
			}
			strerr_warn4("valias_track: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
		if (!(res = in_mysql_store_result(&mysql[1])))
			return (-1);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (alias) {
			if (!stralloc_copys(alias, row[0]) || !stralloc_0(alias))
				die_nomem();
			alias->len--;
		}
		if (domain) {
			if (!stralloc_copys(domain, row[1]) || !stralloc_0(domain))
				die_nomem();
			domain->len--;
		}
		return (0);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return (1);
}

char           *
valias_select_all(stralloc *alias, stralloc *domain)
{
	int             err;
	static stralloc SqlBuf = {0};
	const char     *real_domain;
	MYSQL_ROW       row;
	static MYSQL_RES *res;

	if (!res) {
		if ((err = iopen((char *) 0)) != 0)
			return ((char *) 0);
		if (domain && domain->len) {
			if (!(real_domain = get_real_domain(domain->s)))
				real_domain = domain->s;
			if (!stralloc_copyb(&SqlBuf, "select high_priority alias, domain, valias_line from valias where domain=\"", 74) ||
					!stralloc_cats(&SqlBuf, real_domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		} else {
			if (!stralloc_copyb(&SqlBuf, "select high_priority alias, domain, valias_line from valias", 59) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		}
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return ((char *) 0);
			}
			strerr_warn4("valias_select_all: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((char *) 0);
		}
		if (!(res = in_mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (alias) {
			if (!stralloc_copys(alias, row[0]) ||
					!stralloc_0(alias))
				die_nomem();
			alias->len--;
		}
		if (domain && !domain->len) {
			if (!stralloc_copys(domain, row[1]) ||
					!stralloc_0(domain))
				die_nomem();
			domain->len--;
		}
		return (row[2]);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif /*- #ifdef VALIAS */
