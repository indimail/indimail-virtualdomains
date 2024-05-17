/*
 * $Log: sql_active.c,v $
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2021-02-06 22:41:51+05:30  Cprogrammer
 * fixed loop
 *
 * Revision 1.3  2021-02-02 22:16:48+05:30  Cprogrammer
 * use create_table() to create table
 *
 * Revision 1.2  2019-04-22 23:15:10+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.1  2019-04-15 12:37:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_active.c,v 1.5 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "getpeer.h"
#include "variables.h"
#include "parse_quota.h"
#include "vset_lastauth.h"
#include "check_quota.h"
#include "fstabChangeCounters.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("sql_active: out of memory", 0);
	_exit(111);
}

int
sql_active(struct passwd *pw, const char *domain, int type)
{
	char           *table1 = NULL, *table2 = NULL;
	int             row_count, i;
	mdir_t          quota;
	static stralloc SqlBuf = {0}, Dir = {0};

	userNotFound = 0;
	if(site_size == LARGE_SITE || !domain || !*domain || iopen((char *) 0))
		return (1);
	if(type != FROM_INACTIVE_TO_ACTIVE && type != FROM_ACTIVE_TO_INACTIVE)
		return (1);
	if(type == FROM_INACTIVE_TO_ACTIVE) {
		table1  = inactive_table;
		table2 = default_table;
	} else
	if(type == FROM_ACTIVE_TO_INACTIVE) {
		table1 = default_table;
		table2  = inactive_table;
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into ", 25) ||
			!stralloc_cats(&SqlBuf, table2) ||
			!stralloc_catb(&SqlBuf, " select high_priority * from ", 29) ||
			!stralloc_cats(&SqlBuf, table1) ||
			!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
			!stralloc_cats(&SqlBuf, pw->pw_name) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	for (i = 0;i < 2; i++) {
		if (mysql_query(&mysql[1], SqlBuf.s)) { /*- insert record in table2 */
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				if (create_table(ON_LOCAL, table2, site_size == LARGE_SITE ? LARGE_TABLE_LAYOUT : SMALL_TABLE_LAYOUT))
					return (1);
				continue;
			}
			strerr_warn4("sql_active: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		} else
			break;
	}
	row_count = in_mysql_affected_rows(&mysql[1]);
	if(row_count == -1 || !row_count)
		return (1);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
			!stralloc_cats(&SqlBuf, table1) ||
			!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
			!stralloc_cats(&SqlBuf, pw->pw_name) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) { /*- remove record from table1 */
		strerr_warn4("sql_active: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
	row_count = in_mysql_affected_rows(&mysql[1]);
#ifdef DELETE_AUTH_RECORD
	if(type == FROM_ACTIVE_TO_INACTIVE) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from lastauth where user = \"", 48) ||
				!stralloc_cats(&SqlBuf, pw->pw_name) ||
				!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_catb(&SqlBuf, "\" and (service = \"pop3\" or service=\"imap\")", 42) ||
			!stralloc_0(&SqlBuf))
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_active: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (1);
		}
	}
#endif
	if(row_count == -1 || !row_count)
		return (1);
	if(type == FROM_ACTIVE_TO_INACTIVE)
		is_inactive = 1;
	else
	if(type == FROM_INACTIVE_TO_ACTIVE)
		is_inactive = 0;
	if (!stralloc_copys(&Dir, pw->pw_dir) ||
			!stralloc_catb(&Dir, "/Maildir", 8) ||
			!stralloc_0(&Dir))
		die_nomem();
#ifdef USE_MAILDIRQUOTA
	if ((quota = parse_quota(pw->pw_shell, 0)) == -1) {
		strerr_warn3("sql_active: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
		return (-1);
	}
	vset_lastauth(pw->pw_name, domain, ((type == FROM_ACTIVE_TO_INACTIVE) ? "INAC" : "ACTI"), GetPeerIPaddr(),
			pw->pw_gecos, (type == FROM_ACTIVE_TO_INACTIVE) ? 0 : check_quota(Dir.s, 0));
#else
	scan_ulong(pw->pw_shell, (unsigned int *) &quota);
	vset_lastauth(pw->pw_name, domain, ((type == FROM_ACTIVE_TO_INACTIVE) ? "INAC" : "ACTI"), GetPeerIPaddr(),
			pw->pw_gecos, (type == FROM_ACTIVE_TO_INACTIVE) ? 0 : check_quota(Dir.s));
#endif
	if(type == FROM_ACTIVE_TO_INACTIVE)
		fstabChangeCounter(pw->pw_dir, 0, -1, 0 - quota);
	else
	if(type == FROM_INACTIVE_TO_ACTIVE)
		fstabChangeCounter(pw->pw_dir, 0, 1, quota);
	return (0);
}
#endif
