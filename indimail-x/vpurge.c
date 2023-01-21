/*
 * $Log: vpurge.c,v $
 * Revision 1.1  2019-04-18 08:33:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vpurge.c,v 1.1 2019-04-18 08:33:36+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef USE_MYSQL
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <substdio.h>
#include <subfd.h>
#include <scan.h>
#endif
#include "common.h"
#include "variables.h"
#include "indimail.h"
#include "sql_getpw.h"
#include "vquota_select.h"
#include "user_over_quota.h"
#include "purge_files.h"
#include "recalc_quota.h"
#include "check_quota.h"
#include "vset_lastdeliver.h"

static void
die_nomem()
{
	strerr_warn1("vpurge: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	int             err, days, free_space;
	static stralloc user = {0}, domain = {0}, maildir = {0};
	struct passwd  *pw;
	mdir_t          quota;
	time_t          timestamp;

	if (argc != 4) {
		strerr_warn1("usage: vpurge domain days_over_quota min_free_space", 0);
		return (1);
	}
	if (!stralloc_copys(&domain, argv[1]) || !stralloc_0(&domain))
		die_nomem();
	domain.len--;
	scan_ulong(argv[2], (unsigned long *) &timestamp);
	timestamp = time(0) - (timestamp * 86400);
	scan_int(argv[3], &free_space);
	for (;;) {
		if ((err = vquota_select(&user, &domain, &quota, &timestamp)) == -1)
			return (1);
		else
		if (!err)
			break;
		subprintfe(subfdout, "vpurge", "%s@%s %ld %s", user.s, domain.s, quota, ctime(&timestamp));
		if (!(pw = sql_getpw(user.s, domain.s))) {
			if (userNotFound)
				strerr_warn5("sql_getpw: ", user.s, "@", domain.s, ": No such user", 0);
			else
				strerr_warn5("sql_getpw: ", user.s, "@", domain.s, ": temporary problem with db", 0);
			continue;
		}
		if (!stralloc_copys(&maildir, pw->pw_dir) ||
				!stralloc_catb(&maildir, "/Maildir", 8) ||
				!stralloc_0(&maildir))
			die_nomem();
		if (user_over_quota(maildir.s, pw->pw_shell, free_space)) {
			for (days = 30; days >= 0; days -= 5) {
				subprintfe(subfdout, "vpurge", "Purging files greater than %d days\n", days);
				purge_files(maildir.s, days);
#ifdef USE_MAILDIRQUOTA
				recalc_quota(maildir.s, 0, 0, 0, 2);
				quota = check_quota(maildir.s, 0);
#else
				recalc_quota(maildir.s, 2);
				quota = check_quota(maildir.s);
#endif
				if (user_over_quota(maildir.s, pw->pw_shell, free_space)) {
					subprintfe(subfdout, "vpurge", "user %s@%s still over quota %ld\n",
							user.s, domain.s, quota);
					continue;
				}
				subprintfe(subfdout, "vpurge", "removing bounce for user %s@%s\n",
						user.s, domain.s);
				vset_lastdeliver(user.s, domain.s, 0);
				break;
			}
		} else
			vset_lastdeliver(user.s, domain.s, 0);
	} /*- for(;;) */
	flush("vpurge");
	return(0);
}
#else
int
main()
{
	strerr_warn1("indimail not configure with --enable-mysql=y", 0);
	return(0);
}
#endif
