/*
 * $Log: vfstab_delete.c,v $
 * Revision 1.1  2019-04-15 12:55:36+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#endif
#include "variables.h"
#include "open_master.h"
#include "iopen.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_delete.c,v 1.1 2019-04-15 12:55:36+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vfstab_delete: out of memory", 0);
	_exit(111);
}

int
vfstab_delete(char *filesystem, char *mdahost)
{
	int             err, i;
	static stralloc SqlBuf = {0};

#ifdef CLUSTERED_SITE
	i = 0;
	if (open_master()) {
		strerr_warn1("vfstab_insert: failed to open master db", 0);
		return (-1);
	}
#else
	i = 1;
	if (iopen(0)) {
		strerr_warn1("vfstab-insert: failed to open local db", 0);
		return (-1);
	}
#endif
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from fstab where filesystem=\"", 49) ||
			!stralloc_cats(&SqlBuf, filesystem) ||
			!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
			!stralloc_cats(&SqlBuf, mdahost) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[i], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
			create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
			strerr_warn1("vfstab_delete: No rows selected", 0);
			return (1);
		} else {
			strerr_warn4("vfstab_delete: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[i])) == -1) {
		strerr_warn2("vfstab_delete: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[i]), 0);
		return (-1);
	}
	if (!err) {
		strerr_warn1("vfstab_delete: No rows selected", 0);
		return (1);
	}
	return (0);
}
