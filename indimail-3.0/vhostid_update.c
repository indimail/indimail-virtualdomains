/*
 * $Log: vhostid_update.c,v $
 * Revision 1.1  2019-04-11 08:58:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_update.c,v 1.1 2019-04-11 08:58:08+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_CONFIG_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
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
	strerr_warn1("vhostid_update: out of memory", 0);
	_exit(111);
}

int
vhostid_update(char *hostid, char *ipaddr)
{
	static stralloc SqlBuf = {0};

	if (open_master()) {
		strerr_warn1("vhostid_update: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority host_table set ipaddr=\"", 43) ||
			!stralloc_cats(&SqlBuf, ipaddr) ||
			!stralloc_catb(&SqlBuf, "\" where host=\"", 14) ||
			!stralloc_cats(&SqlBuf, hostid) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			strerr_warn2("vhostid_update: No rows selected for hostid ", hostid, 0);
			if (create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
				return (-1);
			return (1);
		}
		strerr_warn4("vhostid_update: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (in_mysql_affected_rows(&mysql[0]) == -1) {
		strerr_warn2("vhostid_update: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
