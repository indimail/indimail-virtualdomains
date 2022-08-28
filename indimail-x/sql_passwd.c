/*
 * $Log: sql_passwd.c,v $
 * Revision 1.3  2022-08-28 12:01:49+05:30  Cprogrammer
 * set scram field to NULL when not given
 *
 * Revision 1.2  2022-08-05 21:15:25+05:30  Cprogrammer
 * added scram argument to update scram password
 *
 * Revision 1.1  2019-04-14 22:55:58+05:30  Cprogrammer
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
#include <fmt.h>
#endif
#include "get_indimailuidgid.h"
#include "variables.h"
#include "get_assign.h"
#include "iopen.h"
#include "munch_domain.h"
#include "variables.h"
#include "sql_getpw.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_passwd.c,v 1.3 2022-08-28 12:01:49+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_passwd: out of memory", 0);
	_exit(111);
}

int
sql_passwd(char *user, char *domain, char *pass, char *scram)
{
	char           *tmpstr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             err;
	uid_t           myuid, uid;
	static stralloc SqlBuf = {0};

	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	if (!get_assign(domain, 0, &uid, 0))
		uid = indimailuid;
	myuid = geteuid();
	if (myuid != indimailuid && myuid != 0) {
		if (uid == indimailuid) {
			strnum1[fmt_uint(strnum1,indimailuid)] = 0;
			strerr_warn3("id should be ", strnum1, " or root", 0);
		} else {
			strnum1[fmt_uint(strnum1, indimailuid)] = 0;
			strnum2[fmt_uint(strnum2, uid)] = 0;
			strerr_warn5("sql_passwd: id should be ", strnum1, ", ", strnum2, " or root", 0);
		}
		return (-1);
	}
	if (iopen((char *) 0))
		return (-1);
	if (site_size == LARGE_SITE) {
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
	} else
		tmpstr = default_table;
	if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
			!stralloc_cats(&SqlBuf, tmpstr) ||
			!stralloc_catb(&SqlBuf, " set pw_passwd = \"", 18) ||
			!stralloc_cats(&SqlBuf, pass))
		die_nomem();
	if (scram) {
		if (!stralloc_catb(&SqlBuf, "\", scram = \"", 12) ||
			!stralloc_cats(&SqlBuf, scram) ||
			!stralloc_append(&SqlBuf, "\""))
		die_nomem();
	} else
	if (!stralloc_catb(&SqlBuf, "\", scram = NULL", 15))
		die_nomem();

	if (!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
			!stralloc_cats(&SqlBuf, user))
		die_nomem();
	if (site_size == SMALL_SITE) {
		if (!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain))
			die_nomem();
	}
	if (!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
		die_nomem();

	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_passwd: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	err = in_mysql_affected_rows(&mysql[1]);
	if (!err && site_size == SMALL_SITE) {
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " set pw_passwd = \"", 18) ||
				!stralloc_cats(&SqlBuf, pass))
			die_nomem();
		if (scram) {
			if (!stralloc_catb(&SqlBuf, "\", scram = \"", 12) ||
				!stralloc_cats(&SqlBuf, scram))
			die_nomem();
		}
		if (!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_passwd: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
		err = in_mysql_affected_rows(&mysql[1]);
	}
#ifdef QUERY_CACHE
	if (err == 1)
		sql_getpw_cache(0);
#endif
	return (1);
}
