/*
 * $Log: addusercntrl.c,v $
 * Revision 1.1  2019-04-14 22:55:21+05:30  Cprogrammer
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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#include <stralloc.h>
#endif
#include "create_table.h"
#include "open_master.h"
#include "is_distributed_domain.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: addusercntrl.c,v 1.1 2019-04-14 22:55:21+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("addusercntrl: out of memory", 0);
	_exit(111);
}

/*
 * To add an entry into the Location DB.
 *  2 - User Exists
 *  1 - Mysql Error (or) Assignment Error
 *  0 - Success
 */

int
addusercntrl(const char *user, const char *domain, const char *hostid, const char *pass, int force)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];
	int             err, i;

	if (!user || !*user || !domain || !*domain || !pass || !*pass)
		return (1);
	/*
	 *  Check if Domain is distributed or not, by checking table hostcntrl
	 */
	if (!force) {
		if ((err = is_distributed_domain(domain)) == -1)
			return (1);
		if (!err)
			return (0);
	}
	if (open_master()) {
		strerr_warn1("addusercntrl: Failed to open master db", 0);
		return (1);
	}
	strnum[i = fmt_ulong(strnum, time(0))] = 0;
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into ", 25) ||
			!stralloc_cats(&SqlBuf, cntrl_table) ||
			!stralloc_catb(&SqlBuf, " (pw_name, pw_domain, pw_passwd, host, timestamp) ", 50) ||
			!stralloc_catb(&SqlBuf, "values (\"", 9) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, pass) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, hostid) ||
			!stralloc_catb(&SqlBuf, "\", FROM_UNIXTIME(", 17) ||
			!stralloc_catb(&SqlBuf, strnum, i) ||
			!stralloc_catb(&SqlBuf, "))", 2) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if ((err = in_mysql_errno(&mysql[0])) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
				return (1);
			if (!mysql_query(&mysql[0], SqlBuf.s))
				return (0);
		}
		strerr_warn4("addusercntrl: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		if (err == ER_DUP_ENTRY)
			return (2);
		return (1);
	}
	return (0);
}
#endif
