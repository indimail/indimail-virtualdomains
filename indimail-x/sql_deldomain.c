/*
 * $Log: sql_deldomain.c,v $
 * Revision 1.3  2022-10-27 17:28:29+05:30  Cprogrammer
 * refactored sql code into do_sql()
 *
 * Revision 1.2  2019-07-02 17:06:02+05:30  Cprogrammer
 * open master db before deleting from hostcntrl
 *
 * Revision 1.1  2019-04-14 22:52:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "is_distributed_domain.h"
#include "munch_domain.h"
#include "variables.h"
#include "get_local_hostid.h"
#include "sql_updateflag.h"
#include "sql_getall.h"
#include "vdelfiles.h"
#include "delusercntrl.h"
#include "valias_delete_domain.h"
#include "vsmtp_delete_domain.h"
#include "open_master.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_deldomain.c,v 1.3 2022-10-27 17:28:29+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <mysqld_error.h>

static void
die_nomem()
{
	strerr_warn1("sql_deldomaain: out of memory", 0);
	_exit(111);
}

static int
do_sql(const char *domain, const char *table, stralloc *sqlbuf)
{
	if (site_size == LARGE_SITE) {
		if (!stralloc_copyb(sqlbuf, "drop table ", 11) ||
				!stralloc_cats(sqlbuf, table))
			die_nomem();
	} else {
		if (!stralloc_copyb(sqlbuf, "delete low_priority from ", 25) ||
				!stralloc_cats(sqlbuf, table) ||
				!stralloc_catb(sqlbuf, " where pw_domain = \"", 20) ||
				!stralloc_cats(sqlbuf, domain) ||
				!stralloc_append(sqlbuf, "\""))
			die_nomem();
	}
	if (!stralloc_0(sqlbuf))
		die_nomem();
	if (mysql_query(&mysql[1], sqlbuf->s)) {
		strerr_warn4("sql_deldomain: ", sqlbuf->s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return -1;
	}
	return 0;
}

int
sql_deldomain(const char *domain)
{
	char           *tmpstr;
	struct passwd  *pw;
	int             is_dist, err;
    static stralloc SqlBuf = {0};

	if (iopen((char *) 0))
		return (1);
	if ((is_dist = is_distributed_domain(domain)) == -1) {
		strerr_warn3("sql_deldomain: Unable to verify ", domain, " as a distributed domain", 0);
		return (1);
	}
	for (err = 0, pw = sql_getall(domain, 1, 0); pw; pw = sql_getall(domain, 0, 0)) {
		if (verbose) {
			out("sql_deldomain", "Removing user ");
			out("sql_deldomain", pw->pw_name);
			out("sql_deldomain", "\n");
			flush("sql_deldomain");
		}
		vdelfiles(pw->pw_dir, pw->pw_name, domain);
#ifdef CLUSTERED_SITE
		if (is_dist) {
			if (sql_updateflag(pw->pw_name, domain, DEL_FLAG) || delusercntrl(pw->pw_name, domain, 0))
				continue;
		}
#endif
	}
	if (site_size == LARGE_SITE) {
		if ((err = do_sql(domain, munch_domain(domain), &SqlBuf)) == -1)
			return -1;
	} else {
		if ((err = do_sql(domain, default_table, &SqlBuf)) == -1)
			return -1;
		else
		if (!err && (err = do_sql(domain, inactive_table, &SqlBuf)) == -1)
			return -1;
	}

#ifdef ENABLE_AUTH_LOGGING
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from lastauth where domain = \"", 50) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s) && in_mysql_errno(&mysql[1]) != ER_NO_SUCH_TABLE) {
		strerr_warn4("sql_deldomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		err = (err ? 1 : 0);
	}
#endif
#ifdef VALIAS
	err = valias_delete_domain(domain) ? 1 : err;
#endif
#ifdef CLUSTERED_SITE
	if (is_dist) {
		err = (vsmtp_delete_domain(domain) ? 1 : err);
		if (!(tmpstr = get_local_hostid())) {
			if (!err)
				err = 1;
			strerr_warn1("sql_deldomain: get_local_hostid: Unable to get hostid", 0);
			return (err);
		}
		if (open_master()) {
			strerr_warn1("sql_deldomain: failed to open master db", 0);
			return (-1);
		}
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, cntrl_table) ||
				!stralloc_catb(&SqlBuf, " where pw_domain = \"", 20) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_catb(&SqlBuf, "\" and host = \"", 14) ||
				!stralloc_cats(&SqlBuf, tmpstr) ||
				!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s) && in_mysql_errno(&mysql[0]) != ER_NO_SUCH_TABLE) {
			strerr_warn4("sql_deldomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			err = (err ? 1 : 0);
		}
	}
#endif
	return (err);
}
