/*
 * $Log: make_user_dir.c,v $
 * Revision 1.2  2021-09-11 13:39:42+05:30  Cprogrammer
 * use getEnvConfig for domain directory
 *
 * Revision 1.1  2019-04-18 08:31:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "vmake_maildir.h"
#include "open_big_dir.h"
#include "next_big_dir.h"
#include "close_big_dir.h"
#include "get_Mplexdir.h"
#include "backfill.h"
#include "dir_control.h"
#include "SendWelcomeMail.h"

#ifndef	lint
static char     sccsid[] = "$Id: make_user_dir.c,v 1.2 2021-09-11 13:39:42+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("make_user_dir: out of memory", 0);
	_exit(111);
}

/*
 * figure out where to put the user and
 * make the directories if needed
 */
char           *
make_user_dir(const char *username, const char *domain, uid_t uid, gid_t gid, int users_per_level)
{
	char           *tmpstr, *fname;
	static stralloc tmp1 = {0}, tmp2 = {0};
	int             fd;

	if (!*username)
		return ((char *) 0);
	if (!domain || !*domain) {
		getEnvConfigStr(&tmpstr, "DOMAINDIR", DOMAINDIR);
		if (!stralloc_copys(&tmp1, tmpstr) ||
				!stralloc_append(&tmp1, "/") ||
				!stralloc_catb(&tmp1, "/users", 7) || !stralloc_0(&tmp1))
			die_nomem();
	} else
	if (!stralloc_copys(&tmp1, (char *) get_Mplexdir(username, domain, create_flag, uid, gid)) ||
			!stralloc_0(&tmp1))
		die_nomem();
	tmp1.len--;
	if (!(tmpstr = backfill(username, domain, tmp1.s, 1))) {
		if (!(fname = open_big_dir(username, domain, tmp1.s))) {
			strerr_warn1("make_user_dir: open_big_dir: ", &strerr_sys);
			return ((char *) 0);
		}
		tmpstr = next_big_dir(uid, gid, users_per_level);
		close_big_dir(fname, domain, uid, gid);
	}
	if (tmpstr && *tmpstr) {
		if (!stralloc_copy(&tmp2, &tmp1) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cats(&tmp2, tmpstr) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cats(&tmp2, username) ||
				!stralloc_0(&tmp2))
			die_nomem();
	} else {
		if (!stralloc_copy(&tmp2, &tmp1) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cats(&tmp2, username) ||
				!stralloc_0(&tmp2))
			die_nomem();
	}
	if (!domain || !*domain || create_flag) {
		if (vmake_maildir(tmp2.s, uid, gid, domain) && errno != EEXIST) {
			strerr_warn3("make_user_dir: mkdir: ", tmp2.s, ": ", &strerr_sys);
			dec_dir_control(tmp2.s, username, domain, uid, gid);
			return ((char *) 0);
		}
		if (!domain || !*domain) {
			if (!stralloc_copy(&tmp1, &tmp2) ||
					!stralloc_catb(&tmp1, "/.qmail", 7) || !stralloc_0(&tmp1))
				die_nomem();
			tmp1.len--;
			if ((fd = open_trunc(tmp1.s)) == -1) {
				dec_dir_control(tmp2.s, username, domain, uid, gid);
				strerr_die3sys(111, "make_user_dir: open: ", tmp1.s, ": ");
			}
			if (write(fd, tmp1.s, tmp1.len) == -1)
				strerr_die3sys(111, "make_user_dir: write: ", tmp1.s, ": ");
			if (write(fd, "/Maildir/\n", 10) == -1)
				strerr_die3sys(111, "make_user_dir: write: ", tmp1.s, ": ");
			close(fd);
			if (chown(tmp1.s, uid, gid)) {
				dec_dir_control(tmp1.s, username, domain, uid, gid);
				strerr_die3sys(111, "make_user_dir: chown: ", tmp1.s, ": ");
				return ((char *) 0);
			}
		} else
		if (create_flag)
			SendWelcomeMail(tmp2.s, username, domain, 0, 0);
	}
	if (*tmpstr)
		return (tmpstr);
	else
		return ("");
}
