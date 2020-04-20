/*
 * $Log: vsmtp_select.c,v $
 * Revision 1.2  2019-04-22 23:21:01+05:30  Cprogrammer
 * removed redundant atoi() statement
 *
 * Revision 1.1  2019-04-10 10:46:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_select.c,v 1.2 2019-04-22 23:21:01+05:30 Cprogrammer Exp mbhangui $";
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
#include <scan.h>
#endif
#include "variables.h"
#include "findhost.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("vsmtp_select: out of memory", 0);
	_exit (111);
}

char           *
vsmtp_select(char *domain, int *Port)
{
	static stralloc SqlBuf = {0}, TmpBuf = {0};
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if (!res) {
		*Port = -1;
		if (open_central_db(0))
			return ((char *) 0);
		if (!stralloc_copyb(&SqlBuf, "select high_priority host, src_host, port from smtp_port where  domain = \"", 74) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
				create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
				return ((char *) 0);
			} else {
				strerr_warn4("vsmtp_select: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
				return ((char *) 0);
			}
		}
		if (!(res = in_mysql_store_result(&mysql[0])))
			return ((char *) 0);
	} 
	if ((row = in_mysql_fetch_row(res))) {
		scan_int(row[2], Port);
		if (!stralloc_copys(&TmpBuf, row[1]) ||
				!stralloc_append(&TmpBuf, " ") ||
				!stralloc_cats(&TmpBuf, row[0]) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		return (TmpBuf.s);
	}
	in_mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	*Port = -1;
	return ((char *) 0);
}

#endif
