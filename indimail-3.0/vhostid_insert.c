/*
 * $Log: vhostid_insert.c,v $
 * Revision 1.1  2019-04-11 08:57:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
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

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_insert.c,v 1.1 2019-04-11 08:57:11+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

static void
die_nomem()
{
	strerr_warn1("vhostid_insert: out of memory", 0);
	_exit(111);
}

int
vhostid_insert(char *hostid, char *ipaddr)
{
	static stralloc SqlBuf = {0};

	if (open_master()) {
		strerr_warn1("vhostid_insert: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into host_table (host, ipaddr) values (\"", 60) ||
			!stralloc_cats(&SqlBuf, hostid) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, ipaddr) ||
			!stralloc_catb(&SqlBuf, "\")", 2) || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
				return (-1);
			if (!mysql_query(&mysql[0], SqlBuf.s))
				return (0);
		}
		strerr_warn4("vhostid_insert: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (in_mysql_affected_rows(&mysql[0]) != 1) {
		strerr_warn2("vhostid_insert: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#endif
