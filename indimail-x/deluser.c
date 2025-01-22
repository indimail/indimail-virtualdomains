/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
/*
#ifndef USE_MAILDIRQUOTA
#include <stdlib.h>
#endif
*/
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include <str.h>
#include <scan.h>
#endif
#include "common.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "is_distributed_domain.h"
#include "open_master.h"
#include "findhost.h"
#include "islocalif.h"
#include "sql_getpw.h"
#include "sql_deluser.h"
#include "parse_quota.h"
#include "vget_lastauth.h"
#include "fstabChangeCounters.h"
#include "create_table.h"
#include "variables.h"
#include "sql_active.h"
#include "del_user_assign.h"
#include "dir_control.h"
#include "vdelfiles.h"
#include "valias_delete.h"

#ifndef	lint
static char     sccsid[] = "$Id: deluser.c,v 1.3 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("deluser: out of memory", 0);
	_exit(111);
}

static int
getch(char *ch)
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die1sys(111, "deluser: getch: unable to read input: ");
	return (r);
}

/*-
 * Function to remove users, depending on the value of remove_db
 * 0 - Only Remove the directory
 * 1 - Remove the directory and entry from indimail database
 * 2 - Remove the directory and move the entry from
 *     indimail to indibak. make the user inactive
 */
int
deluser(const char *user, const char *domain, int remove_db)
{
	struct passwd  *passent;
	static stralloc user_dir = {0}, domain_dir = {0}, tmp = {0}, SqlBuf = {0};
	const char     *real_domain;
	char           *ptr;
	char            ch;
	char            buf[1];
	mdir_t          quota;
#ifdef CLUSTERED_SITE
	char           *mailstore;
	int             err;
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;

	if (!user || !*user || !isalnum((int) *user)) {
		strerr_warn1("Illegal Username", 0);
		return (-1);
	}
	if (remove_db == 2) {
		for (i = 0;rfc_ids[i];i++) {
			if (!str_diffn(user, rfc_ids[i], str_len(rfc_ids[i]) + 1)) {
				strerr_warn1("RFC Ids cannot be made inactive", 0);
				return (-1);
			}
		}
	} else {
		for (i = 0;rfc_ids[i];i++) {
			if (!str_diffn(user, rfc_ids[i], str_len(rfc_ids[i]) + 1)) {
				out("deluser", "Are You sure removing RFC Id ");
				out("deluser", user);
				out("deluser", "@");
				out("deluser", domain);
				out("deluser", " (y/n) - ");
				flush("deluser");
				ch = getch(buf);
				if (ch != 'y' && ch != 'Y')
					return (-1);
				break;
			}
		}
	}
	if (domain && *domain) {
		if (!(real_domain = get_real_domain(domain))) {
			strerr_warn3("deluser: ", domain, " does not exist", 0);
			return (-1);
		} else
		if (!get_assign(real_domain, &domain_dir, &uid, &gid)) {
			strerr_warn3("deluser: real domain ", real_domain, " does not exist", 0);
			return (-1);
		}
#ifdef CLUSTERED_SITE
		if ((err = is_distributed_domain(real_domain)) == -1) {
			strerr_warn3("deluser: ", real_domain, ": unable to verify if domain is distributed", 0);
			return (-1);
		} else
		if (err == 1) {
			if (open_master()) {
				strerr_warn1("deluser: failed to open master db", 0);
				return (-1);
			}
			if (!stralloc_copys(&tmp, user) ||
					!stralloc_append(&tmp, "@") ||
					!stralloc_cats(&tmp, real_domain) ||
					!stralloc_0(&tmp))
				die_nomem();
			if ((mailstore = findhost(tmp.s, 0)) != (char *) 0) {
				i = str_rchr(mailstore, ':');
				if (mailstore[i])
					mailstore[i] = 0;
				for (;*mailstore && *mailstore != ':';mailstore++);
				mailstore++;
			} else {
				if (userNotFound)
					strerr_warn5("deluser: ", user, "@", real_domain, ": no such user", 0);
				else
					strerr_warn1("deluser: error connecting to db", 0);
				return (-1);
			}
			if (!islocalif(mailstore)) {
				strerr_warn7("deluser: ", user, "@", real_domain, " not local (mailstore ", mailstore, ")", 0);
				return (-1);
			}
		}
#endif
		if (!(passent = sql_getpw(user, real_domain))) {
			if (userNotFound)
				strerr_warn5("deluser: ", user, "@", real_domain, ": no such user", 0);
			else
				strerr_warn1("deluser: error connecting to db", 0);
			return (-1);
		}
		if (!stralloc_copys(&user_dir, passent->pw_dir) || !stralloc_0(&user_dir))
			die_nomem();
		user_dir.len--;
		switch(remove_db)
		{
			case 1: /*- Delete User */
				if (sql_deluser(user, real_domain)) {
					strerr_warn4("deluser: failed to remove user ", user, "@", real_domain, 0);
					return (-1);
				}
#ifdef USE_MAILDIRQUOTA
				quota = parse_quota(passent->pw_shell, 0);
#else
				scan_ulong(passent->pw_shell, &quota);
#endif
				if (quota == -1) {
					strerr_warn3("deluser: parse_quota: ", passent->pw_shell, ": ", &strerr_sys);
					return (-1);
				}
#ifdef ENABLE_AUTH_LOGGING
				if (vget_lastauth(passent, real_domain, ACTIV_TIME, 0))
					fstabChangeCounter(passent->pw_dir, 0, -1, 0 - quota);
				if (!stralloc_copyb(&SqlBuf, "delete low_priority from lastauth where user=\"", 46) ||
						!stralloc_cats(&SqlBuf, user) ||
						!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
						!stralloc_cats(&SqlBuf, real_domain) ||
						!stralloc_append(&SqlBuf, "\"") ||
						!stralloc_0(&SqlBuf))
					die_nomem();
				if (mysql_query(&mysql[1], SqlBuf.s)) {
					if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
						if (create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT))
							return (-1);
					} else {
						strerr_warn4("deluser: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
						return (-1);
					}
				}
#endif
				break;
			case 2: /*- Make user inactive */
#ifdef ENABLE_AUTH_LOGGING
				if (sql_active(passent, real_domain, FROM_ACTIVE_TO_INACTIVE)) {
					strerr_warn5("deluser: failed to mark user ", user, "@", real_domain, " as inactive", 0);
					return (-1);
				}
#else
				strerr_warn5("deluser: cannot mark user ", user, "@", real_domain,
						" as inactive.\nIndiMail not configured with ENABLE_AUTH_LOGGING", 0);
				return (-1);
#endif
				break;
		}
		if (remove_db) {
#ifdef VFILTER
			if (!stralloc_copyb(&SqlBuf, "delete low_priority from vfilter where emailid=\"", 48) ||
					!stralloc_cats(&SqlBuf, user) ||
					!stralloc_append(&SqlBuf, "@") ||
					!stralloc_cats(&SqlBuf, real_domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
						return (-1);
				} else {
					strerr_warn4("deluser: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
					return (-1);
				}
			}
#endif
#ifdef ENABLE_AUTH_LOGGING
			if (!stralloc_copyb(&SqlBuf, "delete low_priority from userquota where user=\"", 47) ||
					!stralloc_cats(&SqlBuf, user) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, real_domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
						return(-1);
				} else {
					strerr_warn4("deluser: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
					return (-1);
				}
			}
#endif
#ifdef VALIAS
			/*- Remove forwardings to this email address */
			if (!stralloc_copyb(&SqlBuf, "delete low_priority from valias where valias_line=\"&", 52) ||
					!stralloc_cats(&SqlBuf, user) ||
					!stralloc_append(&SqlBuf, "@") ||
					!stralloc_cats(&SqlBuf, real_domain) ||
					!stralloc_append(&SqlBuf, "\"") ||
					!stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
						return (-1);
				} else {
					strerr_warn4("deluser: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
					return (-1);
				}
			}
			if (valias_delete(user, real_domain, (char *) 0)) {
				strerr_warn4("deluser: valias_delete: failed to remove aliases for user ", user, "@", real_domain, 0);
				return (-1);
			}
#endif
		}
	} else {
		if (!get_assign(user, &user_dir, &uid, &gid)) {
			strerr_warn2(user, ": no such user", 0);
			return (-1);
		}
		if (remove_db && del_user_assign(user, user_dir.s))
			return (-1);
		real_domain = NULL;
	}
	if (remove_db == 1)
		dec_dir_control(user_dir.s, user, real_domain, -1, -1);
	if (!access(user_dir.s, F_OK)) {
		/*
		 * remove the users directory from the file system
		 * and check for error
		 */
		if (vdelfiles(user_dir.s, user, real_domain)) {
			strerr_warn3("failed to remove dir ", user_dir.s, ": ", &strerr_sys);
			return (-1);
		}
	}
	if (!stralloc_copy(&tmp, &domain_dir) ||
			!stralloc_catb(&tmp, "/.qmail-", 8) ||
			!stralloc_cats(&tmp, user) ||
			!stralloc_0(&tmp))
		die_nomem();
	user_dir.len--;
	/* replace all dots with ':' */
	for (ptr = tmp.s + domain_dir.len + 8; *ptr; ptr++) {
		if (*ptr == '.')
			*ptr = ':';
	}
	if (!access(tmp.s, F_OK)) {
		if (verbose) {
			out("deluser", "Removing file ");
			out("deluser", tmp.s);
			out("deluser", "\n");
			flush("deluser");
		}
		if (unlink(tmp.s))
			strerr_warn3("deluser: unlink: ", tmp.s, ": ", &strerr_sys);
	}
	return (0);
}
/*
 * $Log: deluser.c,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2023-03-23 22:06:55+05:30  Cprogrammer
 * skip vdelfiles when directory doesn't exist
 *
 * Revision 1.1  2019-04-14 21:49:41+05:30  Cprogrammer
 * Initial revision
 *
 */
