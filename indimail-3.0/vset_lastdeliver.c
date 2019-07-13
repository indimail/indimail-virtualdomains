/*
 * $Log: vset_lastdeliver.c,v $
 * Revision 1.1  2019-04-15 12:50:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vset_lastdeliver.c,v 1.1 2019-04-15 12:50:04+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "sql_getpw.h"
#include "sql_setpw.h"
#include "create_table.h"
#include "variables.h"
#endif

static void
die_nomem()
{
	strerr_warn1("vset_lastdeliver: out of memory", 0);
	_exit(111);
}

int
vset_lastdeliver(char *user, char *domain, int quota)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
	char            strnum[FMT_ULONG];
#ifdef ENABLE_AUTH_LOGGING
	static stralloc SqlBuf = {0};
	int             err;
#endif

#ifdef ENABLE_AUTH_LOGGING
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_catb(&SqlBuf, "replace ", 8))
		die_nomem();
	if (delayed_insert && !stralloc_catb(&SqlBuf, "delayed ", 8))
		die_nomem();
	if (!stralloc_catb(&SqlBuf, "into userquota set user=\"", 25) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\", domain=\"", 11) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", quota=", 9) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, quota)) ||
			!stralloc_catb(&SqlBuf, ", timestamp=FROM_UNIXTIME(", 26) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, time(0))) ||
			!stralloc_append(&SqlBuf, ")") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
				return (1);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("vset_lastdeliver: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (1);
			}
		} else {
			strerr_warn4("vset_lastdeliver: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		}
	}
#endif
	if ((pw = sql_getpw(user, domain))) {
		int gid;

		gid = pw->pw_gid;
		PwTmp = *pw;
		pw = &PwTmp;
		if (!quota  && (pw->pw_gid & BOUNCE_MAIL))
			pw->pw_gid = 0;
		else
		if (quota)
			pw->pw_gid |= BOUNCE_MAIL;
		if (pw->pw_gid != gid)
			return (sql_setpw(pw, domain));
	}
	return (0);
}
