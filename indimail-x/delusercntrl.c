/*
 * $Log: delusercntrl.c,v $
 * Revision 1.1  2019-04-14 22:44:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CLUSTERED_SITE
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "variables.h"
#include "is_distributed_domain.h"
#include "open_master.h"

#ifndef	lint
static char     sccsid[] = "$Id: delusercntrl.c,v 1.1 2019-04-14 22:44:18+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("delusercntrl: out of memory", 0);
	_exit(111);
}

/*
 * 1 - Mysql Error (or) Assignment Error
 * 0 - Success
 */

int
delusercntrl(char *user, char *domain, int force)
{
	static stralloc SqlBuf = {0};
	int             err;

	if (!user || !*user || !domain || !*domain)
		return (1);
	/*- Check if Domain is distributed or not , by checking hostcntrl table */
	if (!force) {
		if ((err = is_distributed_domain(domain)) == -1)
			return (1);
		if (!err)
			return (0);
	}
	if (open_master()) {
		strerr_warn1("delusercntrl: Failed to open Master Db", 0);
		return (1);
	}
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
			!stralloc_cats(&SqlBuf, cntrl_table) ||
			!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("delusercntrl: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	}
	err = in_mysql_affected_rows(&mysql[0]);
	return (err == -1 ? 1 : 0);
}
#endif
