/*
 * $Log: sql_adduser.c,v $
 * Revision 1.2  2020-04-01 18:58:00+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-20 08:34:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <getEnvConfig.h>
#endif
#include "create_table.h"
#include "iopen.h"
#include "get_assign.h"
#include "vset_lastauth.h"
#include "munch_domain.h"
#include "get_Mplexdir.h"
#include "variables.h"
#include "getpeer.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_adduser.c,v 1.2 2020-04-01 18:58:00+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_adduser: out of memory", 0);
	_exit(111);
}

char           *
sql_adduser(char *user, char *domain, char *pass, char *gecos, char *dir, char *Quota, int apop, int actFlag)
{
	static stralloc dirbuf = {0}, quota = {0}, dom_dir = {0}, SqlBuf = {0};
	char           *domstr, *ptr;
	char            strnum[FMT_ULONG];
#ifdef HARD_QUOTA
	char           *hard_quota;
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;

	if (iopen((char *) 0))
		return ((char *) 0);
	if (Quota && *Quota) {
		if (!stralloc_copys(&quota, Quota) || !stralloc_0(&quota))
			die_nomem();
		quota.len--;
	} else {
#ifdef HARD_QUOTA
		getEnvConfigStr(&hard_quota, "HARD_QUOTA", HARD_QUOTA);
		if (!stralloc_copys(&quota, hard_quota) || !stralloc_0(&quota))
			die_nomem();
		quota.len--;
#else
		if (!stralloc_copyb(&quota, "NOQUOTA", 7) || !stralloc_0(&quota))
			die_nomem();
		quota.len--;
#endif
	}
	domstr = (char *) 0;
	if (!domain || !*domain) {
		if (!stralloc_copys(&dom_dir, DOMAINDIR) ||
				!stralloc_catb(&dom_dir, "/users", 6) || !stralloc_0(&dom_dir))
			die_nomem();
		dom_dir.len--;
	} else {
		if (!get_assign(domain, 0, &uid, &gid)) {
			strerr_warn3("Domain ", domain, " does not exist", 0);
			return ((char *) 0);
		}
		ptr = get_Mplexdir(user, domain, 0, uid, gid);
		if (!stralloc_copys(&dom_dir, ptr) || !stralloc_0(&dom_dir))
			die_nomem();
		dom_dir.len--;
	}
	if (dir && *dir) {
		if (!stralloc_cat(&dirbuf, &dom_dir) ||
				!stralloc_append(&dirbuf, "/") ||
				!stralloc_cats(&dirbuf, dir) ||
				!stralloc_append(&dirbuf, "/") ||
				!stralloc_cats(&dirbuf, user) || !stralloc_0(&dirbuf))
			die_nomem();
		dirbuf.len--;
	} else {
		if (!stralloc_cat(&dirbuf, &dom_dir) ||
				!stralloc_append(&dirbuf, "/") ||
				!stralloc_cats(&dirbuf, user) || !stralloc_0(&dirbuf))
			die_nomem();
		dirbuf.len--;
	}
	if (site_size == LARGE_SITE) {
		if (domain && *domain)
			domstr = munch_domain(domain);
		else
			domstr = MYSQL_LARGE_USERS_TABLE;
		if (!stralloc_copyb(&SqlBuf, "insert low_priority into  ", 26) ||
				!stralloc_cats(&SqlBuf, domstr) ||
				!stralloc_catb(&SqlBuf, " (pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell)", 65) ||
				!stralloc_catb(&SqlBuf, " values (\"", 10) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cats(&SqlBuf, pass) ||
				!stralloc_catb(&SqlBuf, "\", ", 3) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, apop)) ||
				!stralloc_catb(&SqlBuf, ", 0, \"", 6) ||
				!stralloc_cats(&SqlBuf, gecos) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cat(&SqlBuf, &dirbuf) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cat(&SqlBuf, &quota) ||
				!stralloc_catb(&SqlBuf, "\")", 2) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		for(i = 0;rfc_ids[i];i++) {
			if (!str_diffn(user, rfc_ids[i], str_len(rfc_ids[i]) + 1))
				break;
		}
		if (!stralloc_copyb(&SqlBuf, "insert low_priority into  ", 26) ||
				!stralloc_cats(&SqlBuf, rfc_ids[i] || actFlag ? default_table : inactive_table) ||
				!stralloc_catb(&SqlBuf, " (pw_name, pw_domain, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell)", 76) ||
				!stralloc_catb(&SqlBuf, " values (\"", 10) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cats(&SqlBuf, pass) ||
				!stralloc_catb(&SqlBuf, "\", ", 3) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, apop)) ||
				!stralloc_catb(&SqlBuf, ", 0, \"", 6) ||
				!stralloc_cats(&SqlBuf, gecos) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cat(&SqlBuf, &dirbuf) ||
				!stralloc_catb(&SqlBuf, "\", \"", 4) ||
				!stralloc_cat(&SqlBuf, &quota) ||
				!stralloc_catb(&SqlBuf, "\")", 2) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			for(i = 0;rfc_ids[i];i++) {
				if (!str_diffn(user, rfc_ids[i], str_len(rfc_ids[i]) + 1))
					break;
			}
			if (create_table(ON_LOCAL,
				(rfc_ids[i] || actFlag) ? default_table : inactive_table, site_size == LARGE_SITE ? LARGE_TABLE_LAYOUT : SMALL_TABLE_LAYOUT))
				return ((char *) 0);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("sql_adduser: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return ((char *) 0);
			}
		} else {
			strerr_warn4("sql_adduser: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((char *) 0);
		}
	}
#ifdef ENABLE_AUTH_LOGGING
	ptr = GetPeerIPaddr();
	vset_lastauth(user, domain, "add", ptr, gecos, 0);
#endif
	return (dirbuf.s);
}
