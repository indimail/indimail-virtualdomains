/*
 * $Log: vsmtp_insert.c,v $
 * Revision 1.1  2019-04-20 09:09:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_insert.c,v 1.1 2019-04-20 09:09:30+05:30 Cprogrammer Exp mbhangui $";
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
	strerr_warn1("vsmtp_insert: out of memory", 0);
	_exit (111);
}

int
vsmtp_insert(char *host, char *src_host, char *domain, int smtp_port)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (open_master()) {
		strerr_warn1("vsmtp_insert: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into smtp_port (host, src_host, domain, port) values (\"", 75) ||
			!stralloc_cats(&SqlBuf, host) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, src_host) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) smtp_port)) ||
			!stralloc_append(&SqlBuf, ")") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT))
				return (-1);
			if (!mysql_query(&mysql[0], SqlBuf.s))
				return (0);
		}
		strerr_warn4("vsmtp_insert: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
