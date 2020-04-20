/*
 * $Log: sql_getflags.c,v $
 * Revision 1.2  2019-04-22 23:15:28+05:30  Cprogrammer
 * replaced atoi() with scan_uint()
 *
 * Revision 1.1  2019-04-14 23:06:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_getflags.c,v 1.2 2019-04-22 23:15:28+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <alloc.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "variables.h"
#include "indimail.h"

static struct passwd **SqlPtr;

static void
die_nomem()
{
	strerr_warn1("sql_getflags: out of memory", 0);
	_exit(111);
}

void
format_sql(stralloc *sqlbuf, char *table_name, int add_flag, int del_flag, char *domain)
{
	char            strnum[FMT_ULONG];

	if (!stralloc_copyb(sqlbuf, "select pw_name, pw_passwd, pw_uid from ", 39) ||
			!stralloc_cats(sqlbuf, table_name) ||
			!stralloc_catb(sqlbuf, " where pw_uid in (", 18) ||
			!stralloc_catb(sqlbuf, strnum, fmt_uint(strnum, (unsigned int) add_flag)) ||
			!stralloc_catb(sqlbuf, ", ", 2) ||
			!stralloc_catb(sqlbuf, strnum, fmt_uint(strnum, (unsigned int) del_flag)) ||
			!stralloc_catb(sqlbuf, ") and pw_domain=\"", 17) ||
			!stralloc_cats(sqlbuf, domain) ||
			!stralloc_append(sqlbuf, "\"") ||
			!stralloc_0(sqlbuf))
		die_nomem();
	return;
}

struct passwd *
sql_getflags(char *domain, int first)
{
	static int      more, flag;
	int             i, err;
	unsigned long   num;
	static MYSQL_RES *res1, *res2;
	MYSQL_ROW       row;
	static stralloc SqlBuf = {0};

	if (first == 1 || !flag) {
		if ((err = iopen((char *) 0)) != 0)
			return ((struct passwd *) 0);
		format_sql(&SqlBuf, default_table, ADD_FLAG, DEL_FLAG, domain);
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_getflags: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		if (!(res1 = in_mysql_store_result(&mysql[1]))) {
			strerr_warn2("sql_getflags: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		format_sql(&SqlBuf, inactive_table, ADD_FLAG, DEL_FLAG, domain);
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
				if (create_table(ON_LOCAL, inactive_table, SMALL_TABLE_LAYOUT)) {
					in_mysql_free_result(res1);
					res1 = (MYSQL_RES *) 0;
					res2 = (MYSQL_RES *) 0;
					return ((struct passwd *) 0);
				}
				res2 = (MYSQL_RES *) 0;
			} else {
				strerr_warn4("sql_getflags: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				in_mysql_free_result(res1);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				return ((struct passwd *) 0);
			}
		} else
		if (!(res2 = in_mysql_store_result(&mysql[1]))) {
			strerr_warn2("sql_getflags: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
			in_mysql_free_result(res1);
			res1 = (MYSQL_RES *) 0;
			return ((struct passwd *) 0);
		}
		if (!(num = (res2 ? (in_mysql_num_rows(res1) + in_mysql_num_rows(res2)) : in_mysql_num_rows(res1)))) {
			in_mysql_free_result(res1);
			if (res2)
				in_mysql_free_result(res2);
			res1 = (MYSQL_RES *) 0;
			res2 = (MYSQL_RES *) 0;
			return ((struct passwd *) 0);
		}
		if (SqlPtr) {
			for (more = 0; SqlPtr[more]; more++) {
				alloc_free(SqlPtr[more]->pw_name);
				alloc_free(SqlPtr[more]->pw_passwd);
				alloc_free((char *) SqlPtr[more]);
			}
			alloc_free((char *) SqlPtr);
			SqlPtr = (struct passwd **) 0;
		}
		if (!(SqlPtr = (struct passwd **) alloc(sizeof(struct passwd *) * (num + 1)))) {
			strerr_warn1("sql_getflags: alloc: ", &strerr_sys);
			in_mysql_free_result(res1);
			if (res2)
				in_mysql_free_result(res2);
			res1 = (MYSQL_RES *) 0;
			res2 = (MYSQL_RES *) 0;
			return ((struct passwd *) 0);
		}
		for (more = 0;(row = in_mysql_fetch_row(res1));more++) {
			if (!(SqlPtr[more] = (struct passwd *) alloc(sizeof(struct passwd)))) {
				strerr_warn1("sql_getflags: alloc: ", &strerr_sys);
				in_mysql_free_result(res1);
				if (res2)
					in_mysql_free_result(res2);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				for (more--;more != -1;more--)
					alloc_free((char *) SqlPtr[more]);
				alloc_free((char *) SqlPtr);
				SqlPtr = (struct passwd **) 0;
				return ((struct passwd *) 0);
			}
			SqlPtr[more]->pw_name = alloc(i = (str_len(row[0]) + 1));
			str_copyb(SqlPtr[more]->pw_name, row[0], i);
			SqlPtr[more]->pw_passwd = alloc(i = (str_len(row[1]) + 1));
			str_copyb(SqlPtr[more]->pw_passwd, row[1], i);
			scan_uint(row[2], (unsigned int *) &(SqlPtr[more]->pw_uid));
		}
		for (;res2 && (row = in_mysql_fetch_row(res2));more++) {
			if (!(SqlPtr[more] = (struct passwd *) alloc(sizeof(struct passwd)))) {
				strerr_warn1("sql_getflags: alloc: ", &strerr_sys);
				in_mysql_free_result(res1);
				if (res2)
					in_mysql_free_result(res2);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				for (more--;more != -1;more--)
					alloc_free((char *) SqlPtr[more]);
				alloc_free((char *) SqlPtr);
				SqlPtr = (struct passwd **) 0;
				return ((struct passwd *) 0);
			}
			SqlPtr[more]->pw_name = alloc(i = (str_len(row[0]) + 1));
			str_copyb(SqlPtr[more]->pw_name, row[0], i);
			SqlPtr[more]->pw_passwd = alloc(i = (str_len(row[1]) + 1));
			str_copyb(SqlPtr[more]->pw_passwd, row[1], i);
			scan_uint(row[2], (unsigned int *) &(SqlPtr[more]->pw_uid));
		}
		flag++;
		in_mysql_free_result(res1);
		if (res2)
			in_mysql_free_result(res2);
		res1 = (MYSQL_RES *) 0;
		res2 = (MYSQL_RES *) 0;
		SqlPtr[more] = (struct passwd *) 0;
		more = 1;
		return (SqlPtr[0]);
	} 
	if (SqlPtr[more])
		return (SqlPtr[more++]);
	for (more = 0;SqlPtr[more];more++)
	{
		alloc_free(SqlPtr[more]->pw_name);
		alloc_free(SqlPtr[more]->pw_passwd);
		alloc_free((char *) SqlPtr[more]);
	}
	more = flag = 0;
	alloc_free((char *) SqlPtr);
	SqlPtr = (struct passwd **) 0;
	return ((struct passwd *) 0);
}
#endif
