/*
 * $Log: vsmtp_delete_domain.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-15 12:27:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_delete_domain.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "open_master.h"
#include "create_table.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vsmtp_delete_domain: out of memory", 0);
	_exit(111);
}

int
vsmtp_delete_domain(const char *domain)
{
	static stralloc SqlBuf = {0};

	if (open_master()) {
		strerr_warn1("vsmtp_delete_domain: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from smtp_port where domain = \"", 51) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("vsmtp_delete_domain: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
