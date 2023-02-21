/*
 * $Log: vcreate_dir_control.c,v $
 * Revision 1.2  2019-07-04 00:02:44+05:30  Cprogrammer
 * fixed filename
 *
 * Revision 1.1  2019-04-20 08:23:06+05:30  Cprogrammer
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
#include <fmt.h>
#include <strerr.h>
#endif
#include "indimail.h"
#include "get_assign.h"
#include "create_table.h"
#include "update_file.h"
#include "iopen.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: vcreate_dir_control.c,v 1.2 2019-07-04 00:02:44+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vcreate_dir_control: out of memory", 0);
	_exit(111);
}

int
vcreate_dir_control(char *filename, char *domain)
{
	static stralloc SqlBuf = {0}, tmpbuf = {0};
	char            strnum[FMT_ULONG];
	uid_t           uid;
	gid_t           gid;
	char           *ptr = (char *) 0;

	if (domain && *domain && !(ptr = get_assign(domain, 0, &uid, &gid))) {
		strerr_warn2(domain, ": No such domain", 0);
		return (-1);
	}
	if (iopen((char *) 0) != 0)
		return (1);
	if (!stralloc_copyb(&tmpbuf, "dir_control", 11) ||
			!stralloc_cats(&tmpbuf, filename) || !stralloc_0(&tmpbuf))
		die_nomem();
	if (create_table(ON_LOCAL, tmpbuf.s, DIR_CONTROL_TABLE_LAYOUT))
		return (1);
	if (!stralloc_catb(&SqlBuf, "replace low_priority into dir_control", 37) ||
			!stralloc_cats(&SqlBuf, filename) ||
			!stralloc_catb(&SqlBuf, " (domain, cur_users, level_cur, level_max, ", 43) ||
			!stralloc_catb(&SqlBuf, "level_start0, level_start1, level_start2, ", 42) ||
			!stralloc_catb(&SqlBuf, "level_end0, level_end1, level_end2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_mod0, level_mod1, level_mod2, ", 36) ||
			!stralloc_catb(&SqlBuf, "level_index0, level_index1, level_index2, the_dir) ", 51) ||
			!stralloc_catb(&SqlBuf, "values (\"", 9) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, MAX_DIR_LEVELS)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, MAX_DIR_LIST - 1)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, MAX_DIR_LIST - 1)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, MAX_DIR_LIST - 1)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "2, ", 3) ||
			!stralloc_catb(&SqlBuf, "4, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, ", 3) ||
			!stralloc_catb(&SqlBuf, "0, \"\")", 6) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		strerr_warn4("vcreate_dir_control [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (1);
	}
	if (ptr) {
		if (!stralloc_copys(&tmpbuf, ptr) ||
				!stralloc_catb(&tmpbuf, "/.filesystems", 13) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (update_file(tmpbuf.s, filename, 0640))
			return (-1);
		if (!getuid() || !geteuid()) {
			if (chown(tmpbuf.s, uid, gid)) {
				strerr_warn3("vcreate_dir_control: chown: ", tmpbuf.s, ": ", &strerr_sys);
				return (0);
			}
		}
	}
	return (0);
}
