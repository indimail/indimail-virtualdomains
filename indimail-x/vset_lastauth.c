/*
 * $Log: vset_lastauth.c,v $
 * Revision 1.1  2019-04-17 13:03:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vset_lastauth.c,v 1.1 2019-04-17 13:03:18+05:30 Cprogrammer Exp mbhangui $";
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
#include <strerr.h>
#include <fmt.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("sql_setlastauth: out of memory", 0);
	_exit(111);
}

int
vset_lastauth(const char *user, const char *domain, const char *service,
		const char *remoteip, const char *gecos, int quota)
{
	int             err, i, j;
	static stralloc SqlBuf = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if ((err = iopen((char *) 0)) != 0)
		return (err);
	strnum1[i = quota > 0 ? fmt_ulong(strnum1, quota) : fmt_long(strnum1, quota)] = 0;
	strnum2[j = fmt_ulong(strnum2, time(0))] = 0;
	if (!stralloc_copyb(&SqlBuf,
			delayed_insert ?  "replace delayed into lastauth set user=\"" : "replace into lastauth set user=\"",
			delayed_insert ? 40 : 32))
		die_nomem();
	if (!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\", domain=\"", 11) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", service=\"", 12) ||
			!stralloc_cats(&SqlBuf, service) ||
			!stralloc_catb(&SqlBuf, "\", remote_ip=\"", 14) ||
			!stralloc_cats(&SqlBuf, remoteip) ||
			!stralloc_catb(&SqlBuf, "\", quota=", 9) ||
			!stralloc_catb(&SqlBuf, strnum1, i) ||
			!stralloc_catb(&SqlBuf, ", gecos=\"", 9) ||
			!stralloc_cats(&SqlBuf, gecos) ||
			!stralloc_catb(&SqlBuf, "\", timestamp=FROM_UNIXTIME(", 27) ||
			!stralloc_catb(&SqlBuf, strnum2, j) ||
			!stralloc_append(&SqlBuf, ")") || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT))
				return(1);
			if (!mysql_query(&mysql[1], SqlBuf.s))
				return(0);
		}
		strerr_warn4("sql_setlastauth: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return(1);
	}
	return (0);
}
#endif /*- #ifdef ENABLE_AUTH_LOGGING */
