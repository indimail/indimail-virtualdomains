/*
 * $Log: getlastauth.c,v $
 * Revision 1.1  2019-04-14 18:30:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#endif
#include "iopen.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: getlastauth.c,v 1.1 2019-04-14 18:30:35+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("getlastauth: out of memory", 0);
	_exit(111);
}

/*
 * Type 1 : Get Active   User List
 * Type 2 : Get Inactive User List
 */
char  **
getlastauth(char *Domain, int Age, int sortit, int Type, unsigned long *count)
{
	static stralloc SqlBuf = {0};
	static char   **SqlPtr;
	char            strnum[FMT_ULONG];
	int             i;
	time_t          tmval;
	unsigned long   num, more;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	*count = 0;
	if (iopen((char *) 0) != 0) {
		*count = -1;
		return ((char **) 0);
	}
	/*
	 * get the time 
	 */
	tmval = time(NULL);
	/*
	 * subtract the age 
	 */
	tmval = tmval - (86400 * Age);
	strnum[i = fmt_ulong(strnum, (unsigned long) tmval)] = 0;
	if (!stralloc_copyb(&SqlBuf, "select distinct lower(user) from lastauth where domain=\"", 56) ||
			!stralloc_cats(&SqlBuf, Domain) ||
			!stralloc_catb(&SqlBuf, "\" and (service=\"pop3\" or service=\"imap\" or service=\"webm\") and ", 63) ||
			!stralloc_catb(&SqlBuf, Type == 1 ?  "UNIX_TIMESTAMP(timestamp) >= " : "UNIX_TIMESTAMP(timestamp) <  ", 29) ||
			!stralloc_catb(&SqlBuf, strnum, i))
		die_nomem();
	if (sortit == 1 && !stralloc_catb(&SqlBuf, " order by user", 14))
		die_nomem();
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("getlastuath: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		*count = -1;
		return ((char **) 0);
	}
#ifdef LOW_MEM
	if (!(res = mysql_use_result(&mysql[1])))
#else
	if (!(res = in_mysql_store_result(&mysql[1])))
#endif
	{
		strerr_warn2("getlastuath: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		*count = -1;
		return ((char **) 0);
	}
	if (!(num = in_mysql_num_rows(res))) {
		in_mysql_free_result(res);
		return ((char **) 0);
	}
	if (!(SqlPtr = (char **) alloc(sizeof(char *) * (num + 1))))
		die_nomem();
	for (more = 0;(row = in_mysql_fetch_row(res));more++) {
		if (!(SqlPtr[more] = alloc(num = (str_len(row[0]) + 1))))
			die_nomem();
		str_copyb(SqlPtr[more], row[0], num);
	}
#ifdef LOW_MEM
	if (!mysql_eof(res)) {
		strerr_warn3("getlastauth: mysql_eof: ", (char *) in_mysql_error(&mysql[1]), ": ", &strerr_sys);
		in_mysql_free_result(res);
		*count = -1;
		return ((char **) 0);
	}
#endif
	*count = more;
	in_mysql_free_result(res);
	SqlPtr[more] = (char *) 0;
	return (SqlPtr);
}
