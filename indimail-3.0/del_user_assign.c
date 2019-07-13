/*
 * $Log: del_user_assign.c,v $
 * Revision 1.1  2019-04-18 08:23:32+05:30  Cprogrammer
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
#include <fmt.h>
#endif
#include "get_indimailuidgid.h"
#include "variables.h"
#include "getEnvConfig.h"
#include "remove_line.h"
#include "update_newu.h"

#ifndef	lint
static char     sccsid[] = "$Id: del_user_assign.c,v 1.1 2019-04-18 08:23:32+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("del_user_assign: out of memory", 0);
	_exit(111);
}

/*
 * remove a local user from the users/assign file and recompile
 */
int
del_user_assign(char *user, char *dir)
{
	static stralloc tmp1 = {0}, tmp2 = {0}, fname = {0};
	char            strnum[FMT_ULONG];
	int             i;
	char           *assigndir;

	if(indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	if (!stralloc_copyb(&tmp1, "=", 1) ||
			!stralloc_cats(&tmp1, user) ||
			!stralloc_append(&tmp1, ":") ||
			!stralloc_cats(&tmp1, user) ||
			!stralloc_append(&tmp1, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(&tmp1, strnum, i) ||
			!stralloc_append(&tmp1, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(&tmp1, strnum, i) ||
			!stralloc_append(&tmp1, ":") ||
			!stralloc_cats(&tmp1, dir) ||
			!stralloc_catb(&tmp1, ":::", 3) ||
			!stralloc_0(&tmp1))
		die_nomem();

	if (!stralloc_copyb(&tmp2, "+", 1) ||
			!stralloc_cats(&tmp2, user) ||
			!stralloc_append(&tmp2, ":") ||
			!stralloc_cats(&tmp2, user) ||
			!stralloc_append(&tmp2, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(&tmp2, strnum, i) ||
			!stralloc_append(&tmp2, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(&tmp2, strnum, i) ||
			!stralloc_append(&tmp2, ":") ||
			!stralloc_cats(&tmp2, dir) ||
			!stralloc_catb(&tmp2, ":::", 3) ||
			!stralloc_0(&tmp2))
		die_nomem();

	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&fname, assigndir) || !stralloc_0(&fname))
		die_nomem();
	if(remove_line(tmp1.s, fname.s, 0, INDIMAIL_QMAIL_MODE) == -1 || remove_line(tmp2.s, fname.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		return(-1);
	update_newu();
	return (0);
}
