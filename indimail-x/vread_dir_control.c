/*
 * $Log: vread_dir_control.c,v $
 * Revision 1.5  2019-04-22 23:26:23+05:30  Cprogrammer
 * use scan_ulong instead of scan_long
 *
 * Revision 1.4  2019-04-22 23:22:36+05:30  Cprogrammer
 * added scan.h header
 *
 * Revision 1.3  2019-04-22 23:20:00+05:30  Cprogrammer
 * use scan functions instead of atoi, atol
 *
 * Revision 1.2  2019-04-15 22:00:40+05:30  Cprogrammer
 * added dir_control.h for vdir struct definition
 *
 * Revision 1.1  2019-04-15 12:42:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <scan.h>
#endif
#include "dir_control.h"
#include "create_table.h"
#include "iopen.h"
#include "variables.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: vread_dir_control.c,v 1.5 2019-04-22 23:26:23+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vread_dir_control: out of memory", 0);
	_exit(111);
}

int
vread_dir_control(const char *filename, vdir_type *vdir, const char *domain)
{
	int             found = 0;
	static stralloc SqlBuf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (iopen((char *) 0) != 0)
		return (-1);
	if (!stralloc_catb(&SqlBuf, "select high_priority ", 21) ||
			!stralloc_catb(&SqlBuf, "cur_users, level_cur, level_max, ", 33) ||
			!stralloc_catb(&SqlBuf, "level_start0, level_start1, level_start2, ", 42) ||
			!stralloc_catb(&SqlBuf, "level_end0, level_end1, level_end2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_mod0, level_mod1, level_mod2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_index0, level_index1, level_index2, the_dir", 49) ||
			!stralloc_catb(&SqlBuf, " from dir_control", 17) ||
			!stralloc_cats(&SqlBuf, filename) ||
			!stralloc_catb(&SqlBuf, " where domain = \"", 17) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (vcreate_dir_control(filename, domain))
				return(-1);
			init_dir_control(vdir);
			return (0);
		} else {
			strerr_warn4("vread_dir_control: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("vread_dir_control: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
	if ((row = in_mysql_fetch_row(res))) {
		found = 1;
		scan_ulong(row[0], &(vdir->cur_users));
		scan_int(row[1], &(vdir->level_cur));
		scan_int(row[2], &(vdir->level_max));

		scan_int(row[3], &(vdir->level_start[0]));
		scan_int(row[4], &(vdir->level_start[1]));
		scan_int(row[5], &(vdir->level_start[2]));

		scan_int(row[6], &(vdir->level_end[0]));
		scan_int(row[7], &(vdir->level_end[1]));
		scan_int(row[8], &(vdir->level_end[2]));

		scan_int(row[9], &(vdir->level_mod[0]));
		scan_int(row[10], &(vdir->level_mod[1]));
		scan_int(row[11], &(vdir->level_mod[2]));

		scan_int(row[12], &(vdir->level_index[0]));
		scan_int(row[13], &(vdir->level_index[1]));
		scan_int(row[14], &(vdir->level_index[2]));
		str_copyb(vdir->the_dir, row[15], MAX_DIR_NAME);
	}
	in_mysql_free_result(res);
	if (found == 0)
		init_dir_control(vdir);
	return (0);
}
