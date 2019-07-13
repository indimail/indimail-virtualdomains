/*
 * $Log: sql_setpw.c,v $
 * Revision 1.1  2019-04-20 08:43:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#endif
#include "iopen.h"
#include "get_indimailuidgid.h"
#include "get_assign.h"
#include "sql_getpw.h"
#include "pwcomp.h"
#include "copyPwdStruct.h"
#include "munch_domain.h"
#include "indimail.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_setpw.c,v 1.1 2019-04-20 08:43:22+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_setpw: out of memory", 0);
	_exit(111);
}

int
sql_setpw(struct passwd *inpw, char *domain)
{
	static stralloc SqlBuf = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	struct passwd  *pw;
	char           *tmpstr;
	uid_t           myuid;
	uid_t           uid;
	gid_t           gid;
	int             err;

	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	if (!get_assign(domain, 0, &uid, &gid)) {
		uid = indimailuid;
		gid = indimailgid;
	} 
	myuid = geteuid();
	if (myuid != indimailuid && myuid != uid && myuid != 0) {
		if (uid == indimailuid) {
			strnum1[fmt_uint(strnum1, indimailuid)] = 0;
			strerr_warn3("id should be ", strnum1, " or root", 0);
		} else {
			strnum1[fmt_uint(strnum1, indimailuid)] = 0;
			strnum2[fmt_uint(strnum2, uid)] = 0;
			strerr_warn5("id should be ", strnum1, ", ", strnum2, " or root", 0);
		}
		return (-1);
	}
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!(pw = sql_getpw(inpw->pw_name, domain))) {
		if (userNotFound)
			strerr_warn5("sql_setpw: ", inpw->pw_name, "@", domain, ": No such user", 0);
		else
			strerr_warn5("sql_setpw: ", inpw->pw_name, "@", domain, ": temporary database error", 0);
		return (-1);
	}
	if (!pwcomp(pw, copyPwdStruct(inpw)))
		return (0);
	if (site_size == LARGE_SITE) {
		if (!domain || *domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = munch_domain(domain);
	} else
		tmpstr = default_table;
	if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
			!stralloc_cats(&SqlBuf, tmpstr) ||
			!stralloc_catb(&SqlBuf, " set pw_passwd = \"", 18) ||
			!stralloc_cats(&SqlBuf, inpw->pw_passwd) ||
			!stralloc_catb(&SqlBuf, "\", pw_uid = ", 12) ||
			!stralloc_catb(&SqlBuf, strnum1, fmt_uint(strnum1, inpw->pw_uid)) ||
			!stralloc_catb(&SqlBuf, ", pw_gid = ", 11) ||
			!stralloc_catb(&SqlBuf, strnum1, fmt_uint(strnum1, inpw->pw_gid)) ||
			!stralloc_catb(&SqlBuf, ", pw_gecos = \"", 14) ||
			!stralloc_cats(&SqlBuf, inpw->pw_gecos) ||
			!stralloc_catb(&SqlBuf, "\", pw_dir = \"", 13) ||
			!stralloc_cats(&SqlBuf, inpw->pw_dir) ||
			!stralloc_catb(&SqlBuf, "\", pw_shell = \"", 15) ||
			!stralloc_cats(&SqlBuf, inpw->pw_shell) ||
			!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
			!stralloc_cats(&SqlBuf, inpw->pw_name) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("sql_setpw: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	err = in_mysql_affected_rows(&mysql[1]);
	if (!err && site_size == SMALL_SITE) {
		if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " set pw_passwd = \"", 18) ||
				!stralloc_cats(&SqlBuf, inpw->pw_passwd) ||
				!stralloc_catb(&SqlBuf, "\", pw_uid = ", 12) ||
				!stralloc_catb(&SqlBuf, strnum1, fmt_uint(strnum1, inpw->pw_uid)) ||
				!stralloc_catb(&SqlBuf, ", pw_gid = ", 11) ||
				!stralloc_catb(&SqlBuf, strnum1, fmt_uint(strnum1, inpw->pw_gid)) ||
				!stralloc_catb(&SqlBuf, ", pw_gecos = \"", 14) ||
				!stralloc_cats(&SqlBuf, inpw->pw_gecos) ||
				!stralloc_catb(&SqlBuf, "\", pw_dir = \"", 13) ||
				!stralloc_cats(&SqlBuf, inpw->pw_dir) ||
				!stralloc_catb(&SqlBuf, "\", pw_shell = \"", 15) ||
				!stralloc_cats(&SqlBuf, inpw->pw_shell) ||
				!stralloc_catb(&SqlBuf, "\" where pw_name = \"", 19) ||
				!stralloc_cats(&SqlBuf, inpw->pw_name) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				if (!stralloc_copyb(&SqlBuf, "CREATE TABLE IF NOT EXISTS ", 27) ||
						!stralloc_cats(&SqlBuf, inactive_table) ||
						!stralloc_catb(&SqlBuf, " ( ", 3) ||
						!stralloc_cats(&SqlBuf, SMALL_TABLE_LAYOUT) ||
						!stralloc_catb(&SqlBuf, " )", 2) ||
						!stralloc_0(&SqlBuf))
					die_nomem();
				if (mysql_query(&mysql[1], SqlBuf.s)) {
					strerr_warn4("sql_setpw: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
					return (-1);
				}
				err = 0;
			} else {
				strerr_warn4("sql_setpw: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		} else
			err = in_mysql_affected_rows(&mysql[1]);
	}
	if (!err)
		strerr_warn1("0 rows updated", 0);
	if (!err || err == -1)
		err = 1;
	else
	{
#ifdef QUERY_CACHE
		sql_getpw_cache(0);
#endif
		err = 0;
	}
	return (err);
}
