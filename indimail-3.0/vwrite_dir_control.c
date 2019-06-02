/*
 * $Log: vwrite_dir_control.c,v $
 * Revision 1.2  2019-04-15 22:00:52+05:30  Cprogrammer
 * added dir_control.h for vdir struct definition
 *
 * Revision 1.1  2019-04-15 12:48:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#endif
#include "get_assign.h"
#include "update_file.h"
#include "iopen.h"
#include "dir_control.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vwrite_dir_control.c,v 1.2 2019-04-15 22:00:52+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vwrite_dir_control: out of memory", 0);
	_exit(111);
}

int
vwrite_dir_control(char *table_name, vdir_type *vdir, char *domain, uid_t dummy1, gid_t dummy2)
{
	static stralloc SqlBuf = {0}, tmpbuf = {0};
	char            strnum[FMT_ULONG];
	uid_t           uid = 0;
	gid_t           gid = 0;
	char           *ptr = (char *) 0;

	if (domain && *domain && !(ptr = get_assign(domain, 0, &uid, &gid))) {
		strerr_warn2(domain, ": No such domain", 0);
		return (-1);
	}
	if (iopen((char *) 0) != 0)
		return (-1);
	if (!stralloc_catb(&SqlBuf, "replace low_priority into dir_control", 37) ||
			!stralloc_cats(&SqlBuf, table_name) ||
			!stralloc_catb(&SqlBuf, " (domain, cur_users, level_cur, level_max, ", 43) ||
			!stralloc_catb(&SqlBuf, "level_start0, level_start1, level_start2, ", 42) ||
			!stralloc_catb(&SqlBuf, "level_end0, level_end1, level_end2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_mod0, level_mod1, level_mod2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_index0, level_index1, level_index2, the_dir) ", 51) ||
			!stralloc_catb(&SqlBuf, "values (\"", 9) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->cur_users)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_cur)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_max)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_start[0])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_start[1])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_start[2])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_end[0])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_end[1])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_end[2])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_mod[0])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_mod[1])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_mod[2])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_index[0])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_index[1])) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, vdir->level_index[2])) ||
			!stralloc_catb(&SqlBuf, ", \"", 3) ||
			!stralloc_cats(&SqlBuf, vdir->the_dir) ||
			!stralloc_catb(&SqlBuf, "\")", 2) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			vcreate_dir_control(table_name, domain);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn3("vwrite_dir_control: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		} else {
			strerr_warn3("vwrite_dir_control: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if (ptr) {
		if (!stralloc_cats(&tmpbuf, ptr) ||
				!stralloc_catb(&tmpbuf, "/.filesystems", 13) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (update_file(tmpbuf.s, table_name, 0640))
			return (-1);
		if (chown(tmpbuf.s, uid, gid)) {
			strerr_warn3("vwrite_dir_control: chown: ", tmpbuf.s, ": ", &strerr_sys);
			return (0);
		}
	}
	return (0);
}
