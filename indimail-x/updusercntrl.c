/*
 * $Log: updusercntrl.c,v $
 * Revision 1.1  2019-04-14 22:45:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: updusercntrl.c,v 1.1 2019-04-14 22:45:09+05:30 Cprogrammer Exp mbhangui $";
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
#include "variables.h"
#include "is_distributed_domain.h"
#include "create_table.h"
#include "open_master.h"

static void
die_nomem()
{
	strerr_warn1("updusercntrl: out of memory", 0);
	_exit(111);
}

/*-
 * 1 - Mysql Error (or) Assignment Error
 * 0 - Success
 */
int
updusercntrl(char *user, char *domain, char *hostid, int force)
{
	static stralloc SqlBuf = {0};
	int             err;

	if (!user || !*user || !domain || !*domain)
		return (1);
	/*
	 *  Check if Domain is distributed or not , by checking table hostcntrl
	 */
	if (force == 0) {
		if ((err = is_distributed_domain(domain)) == -1)
			return (1);
		else
		if (!err)
			return (0);
	}
	if (open_master()) {
		strerr_warn1("updusercntrl: failed to open master db", 0);
		return (1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
			!stralloc_cats(&SqlBuf, cntrl_table) ||
			!stralloc_catb(&SqlBuf, " set host = \"", 13) ||
			!stralloc_cats(&SqlBuf, hostid) ||
			!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("updusercntrl: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	}
	return (0);
}
#endif
