/*
 * $Log: vfstab_insert.c,v $
 * Revision 1.1  2019-04-15 12:53:53+05:30  Cprogrammer
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
#include <fmt.h>
#endif
#include "variables.h"
#include "open_master.h"
#include "iopen.h"
#include "pathToFilesystem.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_insert.c,v 1.1 2019-04-15 12:53:53+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vfstab_insert: out of memory", 0);
	_exit(111);
}

int
vfstab_insert(char *filesystem, char *host, int status, long max_users, long max_size)
{
	static stralloc SqlBuf = {0};
	char           *ptr;
	char            strnum[FMT_ULONG];
	int             i;

#ifdef CLUSTERED_SITE
	i = 0;
	if (open_master()) {
		strerr_warn1("vfstab_insert: failed to open master db", 0);
		return (-1);
	}
#else
	i = 1;
	if (iopen(0)) {
		strerr_warn1("vfstab_insert: failed to open local db", 0);
		return (-1);
	}
#endif
	if (!(ptr = pathToFilesystem(filesystem))) {
		strerr_warn3("vfstab_insert: ", filesystem, ": Not a filesystem", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf,
			"insert low_priority into fstab (filesystem, host, status, max_users, max_size) values (\"", 88) ||
			!stralloc_cats(&SqlBuf, ptr) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, host) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, max_users)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, max_size)) ||
			!stralloc_append(&SqlBuf, ")") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[i], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
			if (create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT))
				return(-1);
			if (!mysql_query(&mysql[i], SqlBuf.s))
				return(0);
		}
		strerr_warn4("vfstab_insert: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
		return (-1);
	}
	return (0);
}
