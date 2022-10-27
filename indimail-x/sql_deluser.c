/*
 * $Log: sql_deluser.c,v $
 * Revision 1.2  2022-10-27 17:10:06+05:30  Cprogrammer
 * refactored sql code into do_sql()
 *
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
static char     sccsid[] = "$Id: sql_deluser.c,v 1.2 2022-10-27 17:10:06+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_deluser: out of memory", 0);
	_exit(111);
}

static int
do_sql(char *user, char *domain, char *table)
{
	static stralloc SqlBuf = {0};

	if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
			!stralloc_cats(&SqlBuf, table) ||
			!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "\""))
		die_nomem();
	if (site_size == SMALL_SITE &&
			(!stralloc_catb(&SqlBuf, " and pw_domain = \"", 18) ||
			 !stralloc_cats(&SqlBuf, domain) ||
			 !stralloc_append(&SqlBuf, "\"")))
		die_nomem();
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_deluser: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	} 
	return (in_mysql_affected_rows(&mysql[1]));
}

int
sql_deluser(char *user, char *domain)
{
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
	} else
		tmpstr = default_table;

	err = do_sql(user, domain, tmpstr);
	if (site_size == SMALL_SITE && !err)
		err = do_sql(user, domain, inactive_table);
	if (!err || err == -1)
		err = 1;
	else
		err = 0;
	return (err);
}
