/*
 * $Log: del_domain_assign.c,v $
 * Revision 1.1  2019-04-18 08:22:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "indimail.h"
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "remove_line.h"
#include "update_newu.h"

#ifndef	lint
static char     sccsid[] = "$Id: del_domain_assign.c,v 1.1 2019-04-18 08:22:37+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("del_domain_assign: out of memory", 0);
	_exit(111);
}

/*
 * delete a domain from the usrs/assign file
 * input : lots ;)
 * output : 0 = success
 *          less than error = failure
 *
 */
int
del_domain_assign(char *domain, char *dir, gid_t uid, gid_t gid)
{
	static stralloc tmpstr = {0}, fname = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             i, j;
	char           *assigndir;

	strnum1[i = fmt_ulong(strnum1, uid)] = 0;
	strnum2[j = fmt_ulong(strnum2, gid)] = 0;
	if (!stralloc_copyb(&tmpstr, "+", 1) ||
			!stralloc_cats(&tmpstr, domain) ||
			!stralloc_catb(&tmpstr, "-:", 2) ||
			!stralloc_cats(&tmpstr, domain) ||
			!stralloc_append(&tmpstr, ":") ||
			!stralloc_catb(&tmpstr, strnum1, i) ||
			!stralloc_append(&tmpstr, ":") ||
			!stralloc_catb(&tmpstr, strnum2, j) ||
			!stralloc_append(&tmpstr, ":") ||
			!stralloc_cats(&tmpstr, dir) ||
			!stralloc_catb(&tmpstr, ":-::", 4) ||
			!stralloc_0(&tmpstr))
		die_nomem();
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&fname, assigndir) ||
			!stralloc_catb(&fname, "/assign", 7) || !stralloc_0(&fname))
		die_nomem();
	if (remove_line(tmpstr.s, fname.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		return (-1);
	update_newu();
	return (0);
}
