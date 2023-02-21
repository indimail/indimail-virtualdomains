/*
 * $Log: sql_insertaliasdomain.c,v $
 * Revision 1.2  2021-09-11 13:40:52+05:30  Cprogrammer
 * added missing round brace
 *
 * Revision 1.1  2019-04-14 22:51:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_insertaliasdomain.c,v 1.2 2021-09-11 13:40:52+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "open_master.h"
#include "create_table.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("sql_insertaliasdomain: out of memory", 0);
	_exit(111);
}

int
sql_insertaliasdomain(char *old_domain, char *new_domain)
{
#ifdef CLUSTERED_SITE
	int             err;
	static stralloc SqlBuf = {0};
#endif

	if (open_master()) {
		strerr_warn1("sql_insertaliasdomain: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into aliasdomain ( alias, domain ) values ( \"", 65) ||
			!stralloc_cats(&SqlBuf, new_domain) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, old_domain) ||
			!stralloc_catb(&SqlBuf, "\" )", 3) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s) && (err = in_mysql_errno(&mysql[0])) != ER_DUP_ENTRY) {
		if (err == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT))
				return (-1);
			if (!mysql_query(&mysql[0], SqlBuf.s))
				return (0);
		}
		strerr_warn4("sql_insertaliasdomain: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
