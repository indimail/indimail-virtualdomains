/*
 * $Log: vsmtp_delete.c,v $
 * Revision 1.1  2019-04-20 09:07:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_delete.c,v 1.1 2019-04-20 09:07:30+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#endif
#include "open_master.h"
#include "variables.h"
#include "create_table.h"

static void
die_nomem()
{
	strerr_warn1("vsmtp_delete: out of memory", 0);
	_exit (111);
}

int
vsmtp_delete(char *host, char *src_host, char *domain, int port)
{
	int             err;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (open_master()) {
		strerr_warn1("vsmtp_delete: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from smtp_port where host = \"", 49) ||
			!stralloc_cats(&SqlBuf, host) ||
			!stralloc_catb(&SqlBuf, "\" and src_host = \"", 18) ||
			!stralloc_cats(&SqlBuf, src_host) ||
			!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\""))
		die_nomem();
	if (port > 0) {
		if (!stralloc_catb(&SqlBuf, " and port=", 10) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) port)))
			die_nomem();
	}
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			strerr_warn1("vsmtp_delete: No rows selected", 0);
			return (1);
		} 
		strerr_warn4("vsmtp_delete: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("vsmtp_delete: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (err > 0 ? 0 : 1);
}
#endif
