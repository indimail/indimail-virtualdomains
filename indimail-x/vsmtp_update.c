/*
 * $Log: vsmtp_update.c,v $
 * Revision 1.1  2019-04-20 09:17:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_update.c,v 1.1 2019-04-20 09:17:40+05:30 Cprogrammer Exp mbhangui $";
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
	strerr_warn1("vsmtp_update: out of memory", 0);
	_exit (111);
}

int
vsmtp_update(char *host, char *src_host, char *domain, int oldport, int newport)
{
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (open_master()) {
		strerr_warn1("vsmtp_insert: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority smtp_port set port = ", 41) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) newport)) ||
			!stralloc_catb(&SqlBuf, " where host = \"", 15) ||
			!stralloc_cats(&SqlBuf, host) ||
			!stralloc_catb(&SqlBuf, "\" and src_host = \"", 18) ||
			!stralloc_cats(&SqlBuf, src_host) ||
			!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\" and port=", 11) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) oldport)) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			strerr_warn1("vsmtp_update: No rows selected", 0);
			return (0);
		}
		strerr_warn4("vsmtp_update: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
