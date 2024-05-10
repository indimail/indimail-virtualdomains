/*
 * $Log: atrn_access.c,v $
 * Revision 1.2  2024-05-10 11:43:51+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-14 23:09:14+05:30  Cprogrammer
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
#include <str.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "parse_email.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: atrn_access.c,v 1.2 2024-05-10 11:43:51+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("atrn_access: out of memory", 0);
	_exit(111);
}

int
atrn_access(const char *email, const char *domain)
{
	static stralloc SqlBuf = {0}, User = {0}, Domain = {0}, Email = {0};
	int             len, num;
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!email || !*email) {
		Email.len = 0;
		if (select_res) {
			in_mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
		}
		return (0);
	}
	if (Email.len && str_diffn(Email.s, email, Email.len)) {
		Email.len = 0;
		if (select_res) {
			in_mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
		}
	}
	if (!Email.len) {
		if (iopen((char *) 0)) {
			Email.len = 0;
			return (-1);
		}
		if (!stralloc_copys(&Email, email) ||
				!stralloc_0(&Email))
			die_nomem();
		parse_email(email, &User, &Domain);
		if (!stralloc_copyb(&SqlBuf, "select high_priority domain_list from atrn_map where pw_name=\"", 62) ||
				!stralloc_cat(&SqlBuf, &User) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
				!stralloc_cat(&SqlBuf, &Domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			Email.len = 0;
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT);
				return (1);
			}
			strerr_warn4("atrn_access: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
		if (!(select_res = in_mysql_store_result(&mysql[1]))) {
			Email.len = 0;
			return (-1);
		}
		if (!(num = in_mysql_num_rows(select_res))) {
			in_mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
			Email.len = 0;
			return (1);
		}
	}
	in_mysql_data_seek(select_res, 0);
	for (len = str_len(domain);;) {
		if (!(row = in_mysql_fetch_row(select_res)))
			break;
		if (!str_diffn(row[0], domain, len))
			return (0);
	}
	return (1);
}
