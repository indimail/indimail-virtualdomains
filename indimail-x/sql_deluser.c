/*
 * $Log: sql_deluser.c,v $
 * Revision 1.1  2019-04-14 22:52:26+05:30  Cprogrammer
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
#include "is_distributed_domain.h"
#include "delusercntrl.h"
#include "sql_updateflag.h"
#include "munch_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_deluser.c,v 1.1 2019-04-14 22:52:26+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_deluser: out of memory", 0);
	_exit(111);
}

int
sql_deluser(char *user, char *domain)
{
	static stralloc SqlBuf = {0};
	char           *tmpstr;
	int             err;

	if ((err = iopen((char *) 0)) != 0)
		return (1);
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(domain)) == -1) {
		strerr_warn3("sql_deluser: ", domain, ": unable to verify if domain is distributed", 0);
		return (1);
	} else
	if (err == 1) {
		if (sql_updateflag(user, domain, DEL_FLAG))
			return (1);
		if (delusercntrl(user, domain, 0))
			return (1);
	}
#endif
	if (site_size == LARGE_SITE) {
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, tmpstr) ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, default_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_deluser: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	} 
	err = in_mysql_affected_rows(&mysql[1]);
	if (!err && site_size == SMALL_SITE) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_deluser: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		} 
		err = in_mysql_affected_rows(&mysql[1]);
	}
	if (!err || err == -1)
		err = 1;
	else
		err = 0;
	return (err);
}
