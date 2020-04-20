/*
 * $Log: vfstab_status.c,v $
 * Revision 1.1  2019-04-15 13:04:13+05:30  Cprogrammer
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
#include <scan.h>
#include <fmt.h>
#endif
#include "variables.h"
#include "open_master.h"
#include "iopen.h"
#include "pathToFilesystem.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_status.c,v 1.1 2019-04-15 13:04:13+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vfstab_status: out of memory", 0);
	_exit(111);
}

int
vfstab_status(char *filesystem, char *mdahost, int status)
{
	int             err, i;
	static stralloc SqlBuf = {0};
	char           *ptr;
	char            strnum[FMT_ULONG];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

#ifdef CLUSTERED_SITE
	i = 0;
	if (open_master()) {
		strerr_warn1("vfstab_status: failed to open master db", 0);
		return (-1);
	}
#else
	i = 1;
	if (iopen(0)) {
		strerr_warn1("vfstab_status: failed to open local db", 0);
		return (-1);
	}
#endif
	if (!(ptr = pathToFilesystem(filesystem))) {
		strerr_warn3("vfstab_status: ", filesystem, ": Not a filesystem", 0);
		return (-1);
	}
	if (status == -1) {
		if (!stralloc_copyb(&SqlBuf, "select high_priority status from fstab where filesystem=\"", 57) ||
				!stralloc_cats(&SqlBuf, ptr) ||
				!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
				!stralloc_cats(&SqlBuf, mdahost) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[i], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
				create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
				strerr_warn1("vfstab_status: No rows selected", 0);
			} else
				strerr_warn4("vfstab_status: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
			return (-1);
		}
		if (!(res = in_mysql_store_result(&mysql[i]))) {
			strerr_warn2("vfstab_status: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[i]), 0);
			return (-1);
		}
		if (!(row = in_mysql_fetch_row(res))) {
			strerr_warn1("vfstab_status: No rows selected", 0);
			in_mysql_free_result(res);
			return (-1);
		}
		scan_int(row[0], &status);
		status = (status == FS_ONLINE ? FS_OFFLINE : FS_ONLINE);
		in_mysql_free_result(res);
	} /*- if (status == -1) */
	if (!stralloc_copyb(&SqlBuf, "update low_priority fstab set status=", 37) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)) ||
			!stralloc_catb(&SqlBuf, " where filesystem=\"", 19) ||
			!stralloc_cats(&SqlBuf, ptr) ||
			!stralloc_catb(&SqlBuf, "\" and host=\"", 12) ||
			!stralloc_cats(&SqlBuf, mdahost) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[i], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[i]) == ER_NO_SUCH_TABLE) {
			create_table(i == 0 ? ON_MASTER : ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
			strerr_warn1("vfstab_status: No rows selected", 0);
			return (1);
		} else {
			strerr_warn4("vfstab_status: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[i]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[i])) == -1) {
		strerr_warn2("vfstab_status: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[i]), 0);
		return (-1);
	}
	if (!err) {
		strerr_warn1("vfstab_status: No rows selected", 0);
		return (1);
	}
	return (status);
}
