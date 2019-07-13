/*
 * $Log: getindimail.c,v $
 * Revision 1.1  2019-04-15 10:08:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <byte.h>
#endif
#include "iopen.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: getindimail.c,v 1.1 2019-04-15 10:08:01+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("getindimail: out of memory", 0);
	_exit(111);
}

char  **
getindimail(char *domain, int sortit, char **skipGecos, unsigned long *count)
{
	static char   **SqlPtr;
	char          **ptr;
	unsigned long   more, num, dlen;
	static MYSQL_RES *res;
	MYSQL_ROW       row;
	int             skip;
	static stralloc SqlBuf = {0};

	*count = 0;
	if (iopen((char *) 0)) {
		*count = -1;
		return ((char **) 0);
	}
	if (!stralloc_copyb(&SqlBuf, "select lower(pw_name), pw_dir", 29))
		die_nomem();
	if (skipGecos && !stralloc_catb(&SqlBuf, ", pw_gecos", 10))
		die_nomem();
	if (!stralloc_catb(&SqlBuf, " from ", 6) ||
			!stralloc_cats(&SqlBuf, default_table) ||
			!stralloc_catb(&SqlBuf, " where pw_domain=\"", 18) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "/"))
		die_nomem();
	if (sortit == 1 && !stralloc_catb(&SqlBuf, " order by pw_name", 17))
		die_nomem();
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("getindimail: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		*count = -1;
		return ((char **) 0);
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("getindimail: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		*count = -1;
		return ((char **) 0);
	}
	if (!(num = in_mysql_num_rows(res))) {
		in_mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		return ((char **) 0);
	}
	if (!(SqlPtr = (char **) alloc(sizeof(char *) * (num + 1))))
		die_nomem();
	for (more = 0;(row = in_mysql_fetch_row(res));) {
		if (skipGecos) {
			for (skip = 0, ptr = skipGecos;*ptr;ptr++) {
				if (!str_diffn(*ptr, row[2], str_len(*ptr) + 1)) {
					skip = 1;
					break;
				} else
				if (!str_diffn("MailGroup", *ptr, 10) && !byte_diff("MailGroup ", 10, row[2])) {
					skip = 1;
					break;
				}
			}
			if (skip)
				continue;
		}
		if (!(SqlPtr[more] = alloc((num = str_len(row[0])) + (dlen = str_len(row[1])) + 3)))
			die_nomem();
		/*
		 * How pw_name and pw_dir is stored in SqlPtr
		 *
		 * pw_name
		 * NULL
		 * First Char of pw_name
		 * pw_dir
		 */
		str_copyb(SqlPtr[more], row[0], num + 1);
		*(SqlPtr[more] + num + 1) = *(row[0]); /*- Save the first char in username */
		str_copyb(SqlPtr[more] + num + 2, row[1], dlen + 1);
		more++;
	} /*- for (more = 0;(row = in_mysql_fetch_row(res));) */
	*count = more;
	in_mysql_free_result(res);
	SqlPtr[more] = (char *) 0;
	return (SqlPtr);
}
