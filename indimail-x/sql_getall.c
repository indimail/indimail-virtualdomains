/*
 * $Log: sql_getall.c,v $
 * Revision 1.4  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2020-10-18 07:53:54+05:30  Cprogrammer
 * use alloc_re() only for expansion of memory
 *
 * Revision 1.2  2019-05-02 14:38:04+05:30  Cprogrammer
 * reset onum after alloc_free()
 *
 * Revision 1.1  2019-04-15 12:38:51+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#include <alloc.h>
#include <str.h>
#include <scan.h>
#endif
#include "iopen.h"
#include "munch_domain.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_getall.c,v 1.4 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static int      storeSql(int *, int, MYSQL_RES *);
static void     FreeSqlPtr();

static struct passwd **SqlPtr;

static void
die_nomem()
{
	strerr_warn1("sql_getall: out of memory", 0);
	_exit(111);
}

struct passwd *
sql_getall(const char *domain, int first, int sortit)
{
	char           *domstr;
	static int      more, flag;
	int             err;
	unsigned long   num1, num2;
	static MYSQL_RES *res;
	static stralloc SqlBuf = {0};

	if (first == 1 || !flag) {
		if ((err = iopen((char *) 0)) != 0)
			return ((struct passwd *) 0);
		if (SqlPtr)
			FreeSqlPtr();
		/*-- Active Table --------*/
		if (site_size == LARGE_SITE) {
			domstr = munch_domain(domain);
			if (!stralloc_copyb(&SqlBuf,
				"select high_priority pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell from ", 89) ||
					!stralloc_cats(&SqlBuf, domstr))
				die_nomem();
		} else {
			if (!stralloc_copyb(&SqlBuf,
				"select high_priority pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell from ", 89) ||
					!stralloc_cats(&SqlBuf, default_table) ||
					!stralloc_catb(&SqlBuf, " where pw_domain = \"", 20) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_append(&SqlBuf, "\""))
				die_nomem();
		}
		if (sortit == 1 && !stralloc_catb(&SqlBuf, " order by pw_name", 17))
			die_nomem();
		else
		if (!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_getall: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		if (!(res = in_mysql_store_result(&mysql[1]))) {
			strerr_warn2("sql_getall: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		num1 = num2 = more = 0;
		if ((num1 = in_mysql_num_rows(res)) > 0)
			storeSql(&more, num1, res);
		in_mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		/*-- Inactive Table ------*/
		if (!stralloc_copyb(&SqlBuf,
			"select high_priority pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell from ", 89) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " where pw_domain = \"", 20) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
		if (sortit == 1 && !stralloc_catb(&SqlBuf, " order by pw_name", 17))
			die_nomem();
		else
		if (!stralloc_0(&SqlBuf))
			die_nomem();
		if (!mysql_query(&mysql[1], SqlBuf.s)) {
			if (!(res = in_mysql_store_result(&mysql[1]))) {
				strerr_warn2("sql_getall: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
				if (SqlPtr)
					FreeSqlPtr();
				return ((struct passwd *) 0);
			}
			if ((num2 = in_mysql_num_rows(res)) > 0)
				storeSql(&more, num1 + num2, res);
			in_mysql_free_result(res);
			res = (MYSQL_RES *) 0;
		} else
		if (in_mysql_errno(&mysql[1]) != ER_NO_SUCH_TABLE) {
			if (SqlPtr)
				FreeSqlPtr();
			return ((struct passwd *) 0);
		}
		if (!(num1 + num2))
			return ((struct passwd *) 0);
		flag++;
		SqlPtr[more] = (struct passwd *) 0;
		more = 0;
	} /*- if (first == 1 || !flag) */
	if (SqlPtr[more])
		return (SqlPtr[more++]);
	if (SqlPtr)
		FreeSqlPtr();
	more = flag = 0;
	SqlPtr = (struct passwd **) 0;
	return ((struct passwd *) 0);
}

static int
storeSql(int *more, int num, MYSQL_RES *res)
{
	MYSQL_ROW       row;
	int             len;
	static int      onum;

	if (!more || !num || !res) {
		onum = 0;
		return (0);
	}
	if (num + 1 > onum && !alloc_re((void *) &SqlPtr, sizeof(struct passwd *) * onum, sizeof(struct passwd *) * (num + 1))) {
		strerr_warn1("sql_getall: alloc_re: ", &strerr_sys);
		return (1);
	}
	if (num + 1 > onum)
		onum = num + 1;
	for(;(row = in_mysql_fetch_row(res));(*more)++) {
		if (!(SqlPtr[*more] = (struct passwd *) alloc(sizeof(struct passwd)))) {
			strerr_warn1("sql_getall: alloc: ", &strerr_sys);
			return (1);
		}
		SqlPtr[*more]->pw_name = alloc(len = (str_len(row[0]) + 1));
		str_copyb(SqlPtr[*more]->pw_name, row[0], len);
		SqlPtr[*more]->pw_passwd = alloc(len = (str_len(row[1]) + 1));
		str_copyb(SqlPtr[*more]->pw_passwd, row[1], len);
		SqlPtr[*more]->pw_gecos = alloc(len = (str_len(row[4]) + 1));
		str_copyb(SqlPtr[*more]->pw_gecos, row[4], len);
		SqlPtr[*more]->pw_dir = alloc(len = (str_len(row[5]) + 1));
		str_copyb(SqlPtr[*more]->pw_dir, row[5], len);
		SqlPtr[*more]->pw_shell = alloc(len = (str_len(row[6]) + 1));
		str_copyb(SqlPtr[*more]->pw_shell, row[6], len);
		scan_uint(row[2], &(SqlPtr[*more]->pw_uid));
		scan_uint(row[3], &(SqlPtr[*more]->pw_gid));
	}
	SqlPtr[num] = (struct passwd *) 0;
	return (0);
}

static void
FreeSqlPtr()
{
	int             more;
	
	if (SqlPtr) {
		for(more = 0;SqlPtr[more];more++) {
			alloc_free(SqlPtr[more]->pw_name);
			alloc_free(SqlPtr[more]->pw_passwd);
			alloc_free(SqlPtr[more]->pw_gecos);
			alloc_free(SqlPtr[more]->pw_dir);
			alloc_free(SqlPtr[more]->pw_shell);
			alloc_free((char *) SqlPtr[more]);
		}
		alloc_free((char *) SqlPtr);
		SqlPtr = (struct passwd **) 0;
		storeSql(0, 0, 0);
	}
	return;
}
