/*
 * $Log: add_user_assign.c,v $
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2021-09-11 13:24:41+05:30  Cprogrammer
 * use getEnvConfig for domain directory
 *
 * Revision 1.3  2020-10-04 09:25:41+05:30  Cprogrammer
 * BUG: wrong variable passed to do_assign2()
 *
 * Revision 1.2  2020-04-01 18:52:38+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.1  2019-04-18 07:43:37+05:30  Cprogrammer
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
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <open.h>
#include <getEnvConfig.h>
#endif
#include "update_file.h"
#include "update_newu.h"
#include "get_indimailuidgid.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_user_assign.c,v 1.5 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("add_user_assign: out of memory", 0);
	_exit(111);
}

static void
do_assign1(stralloc *s, const char *user, const char *dir)
{
	char            strnum[FMT_ULONG];
	char           *ptr;
	int             i;

	if (!stralloc_copyb(s, "=", 1) ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":") ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	getEnvConfigStr(&ptr, "DOMAINDIR", DOMAINDIR);
	if (!stralloc_cats(s, ptr) || !stralloc_append(s, "/"))
		die_nomem();
	if (dir) {
		if (!stralloc_cats(s, dir) || !stralloc_append(s, "/"))
		die_nomem();
	}
	if (!stralloc_cats(s, user) || !stralloc_0(s))
		die_nomem();
	return;
}

static void
do_assign2(stralloc *s, const char *user, const char *dir)
{
	char            strnum[FMT_ULONG];
	char           *ptr;
	int             i;

	if (!stralloc_copyb(s, "+", 1) ||
			!stralloc_cats(s, user) ||
			!stralloc_catb(s, "-:", 2) ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	getEnvConfigStr(&ptr, "DOMAINDIR", DOMAINDIR);
	if (!stralloc_cats(s, ptr) || !stralloc_append(s, "/"))
		die_nomem();
	if (dir && *dir) {
		if (!stralloc_cats(s, dir) || !stralloc_append(s, "/"))
		die_nomem();
	}
	if (!stralloc_cats(s, user) || !stralloc_0(s))
		die_nomem();
	return;
}

/*
 * add a local user to the users/assign file and compile it
 */
int
add_user_assign(const char *user, const char *dir)
{
	static stralloc filename = {0}, tmpstr1 = {0}, tmpstr2 = {0};
	char           *assigndir;
	int             fd;

	/*
	 * stat assign file, if it's not there create one
	 */
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&filename, assigndir) ||
			!stralloc_catb(&filename, "/assign", 7) ||
			!stralloc_0(&filename))
		die_nomem();
	if (access(filename.s, F_OK)) {
		if ((fd = open_append(filename.s)) == -1) {
			strerr_warn3("add_user_assign: open: ", filename.s, ": ", &strerr_sys);
			return (-1);
		}
		if (write(fd, ".\n", 2) != 2) {
			strerr_warn3("add_user_assign: write: ", filename.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		close(fd);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	do_assign1(&tmpstr1, user, dir);
	do_assign2(&tmpstr2, user, dir);
	if (update_file(filename.s, tmpstr1.s, INDIMAIL_QMAIL_MODE) || update_file(filename.s, tmpstr2.s, INDIMAIL_QMAIL_MODE))
		return (-1);
	update_newu();
	return (0);
}
