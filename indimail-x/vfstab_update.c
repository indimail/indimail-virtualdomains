/*
 * $Log: vfstab_update.c,v $
 * Revision 1.1  2019-04-15 13:04:25+05:30  Cprogrammer
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
static char     sccsid[] = "$Id: vfstab_update.c,v 1.1 2019-04-15 13:04:25+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vfstab_update: out of memory", 0);
	_exit(111);
}

int
vfstab_update(char *filesystem, char *mdahost, long user_quota, long size_quota, int status)
{
	int             err, i;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];
	char           *p;

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
	if (!(p = pathToFilesystem(filesystem))) {
		strerr_warn3("vfstab_insert: ", filesystem, ": Not a filesystem", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority fstab set ", 30))
		die_nomem();
	if (user_quota > 0 && size_quota > 0) {
		if (!stralloc_catb(&SqlBuf, "max_users=", 10) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, user_quota)) ||
				!stralloc_catb(&SqlBuf, ", max_size=", 11) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, size_quota)))
			die_nomem();
		if (status == 0 || status == 1) {
			if (!stralloc_catb(&SqlBuf, ", status=", 9) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)))
				die_nomem();
		}
		if (!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, p) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
	} else
	if (user_quota > 0) {
		if (!stralloc_catb(&SqlBuf, "max_users=", 10) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, user_quota)))
			die_nomem();
		if (status == 0 || status == 1) {
			if (!stralloc_catb(&SqlBuf, ", status=", 9) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)))
				die_nomem();
		}
		if (!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, p) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
	} else
	if (size_quota > 0) {
		if (!stralloc_catb(&SqlBuf, "max_size=", 9) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_long(strnum, size_quota)))
			die_nomem();
		if (status == 0 || status == 1) {
			if (!stralloc_catb(&SqlBuf, ", status=", 9) ||
				!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)))
				die_nomem();
		}
		if (!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
				!stralloc_cats(&SqlBuf, p) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\""))
			die_nomem();
	}
	if (!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[i], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
			create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
			strerr_warn1("vfstab_update: No rows selected", 0);
			return (-1);
		} else {
			strerr_warn4("vfstab_update: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[i])) == -1) {
		strerr_warn2("vfstab_update: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[i]), 0);
		return (-1);
	}
	/*-
	if (!err)
		return (vfstab_insert(p, mdahost, FS_ONLINE, user_quota, size_quota));
	*/
	return (0);
}
