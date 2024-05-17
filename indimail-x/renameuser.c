/*
 * $Id: renameuser.c,v 1.9 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#define XOPEN_SOURCE = 600
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#include <replacestr.h>
#include <get_scram_secrets.h>
#endif
#include "variables.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "sql_setpw.h"
#include "is_distributed_domain.h"
#include "open_master.h"
#include "is_user_present.h"
#include "findhost.h"
#include "islocalif.h"
#include "sql_getpw.h"
#include "iadduser.h"
#include "MoveFile.h"
#include "create_table.h"
#include "valias_select.h"
#include "valias_update.h"
#include "deluser.h"

#ifndef	lint
static char     sccsid[] = "$Id: renameuser.c,v 1.9 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("renameuser: out of memory", 0);
	_exit(111);
}

int
renameuser(stralloc *oldUser, stralloc *oldDomain, stralloc *newUser, stralloc *newDomain)
{
	static stralloc oldDir = {0}, SqlBuf = {0};
	const char     *real_domain;
	char           *ptr, *enc_pass;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
#ifdef VALIAS
	static stralloc User = {0}, oldEmail = {0}, newEmail = {0}, tmp_domain = {0};
#endif
#ifdef CLUSTERED_SITE
	static stralloc tmpbuf = {0};
	char           *mailstore;
#endif
	struct passwd  *pw;
	int             i, err, inactive_flag;
	static stralloc scram = {0};

	if (!oldUser->len || !newUser->len || !oldDomain->len || !newDomain->len || !isalpha((int) *newUser->s)) {
		strerr_warn1("renameuser: Illegal Username/domain", 0);
		return (-1);
	}
	if (newUser->len > MAX_PW_NAME || newDomain->len > MAX_PW_DOMAIN) {
		strnum1[fmt_int(strnum1, MAX_PW_NAME)] = 0;
		strnum2[fmt_int(strnum2, MAX_PW_DOMAIN)] = 0;
		strerr_warn5("renameuser: Name Too Long (name > ", strnum1, " or domain > ", strnum2, ")", 0);
		return (-1);
	}
	if (!(real_domain = get_real_domain(oldDomain->s))) {
		strerr_warn3("renameuser: Domain ", oldDomain->s, " does not exist", 0);
		return (-1);
	} else
	if (!get_assign(real_domain, 0, 0, 0)) {
		strerr_warn3("renameuser: Domain ", real_domain, " does not exist", 0);
		return (-1);
	}
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1) {
		strerr_warn3("renameuser: Unable to verify ", real_domain, " as a distributed domain", 0);
		return (-1);
	} else
	if (err == 1) {
		if (open_master()) {
			strerr_warn1("renameuser: failed to open master db", 0);
			return (-1);
		}
		if (is_user_present(newUser->s, real_domain)) {
			strerr_warn5("User ", newUser->s, "@", real_domain, " exists", 0);
			return (-1);
		}
		if (!stralloc_copy(&tmpbuf, oldUser) ||
				!stralloc_append(&tmpbuf, "@") ||
				!stralloc_cats(&tmpbuf, real_domain) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((mailstore = findhost(tmpbuf.s, 0)) != (char *) 0) {
			i = str_rchr(mailstore, ':');
			if (mailstore[i])
				mailstore[i] = 0;
			else {
				strerr_warn6("renameuser: invalid mailstore [", mailstore, "] for ", oldUser->s, "@", real_domain, 0);
				return (1);
			}
			for(; *mailstore && *mailstore != ':'; mailstore++);
			if  (*mailstore != ':') {
				strerr_warn6("renameuser: invalid mailstore [", mailstore, "] for ", oldUser->s, "@", real_domain, 0);
				return (1);
			}
			mailstore++;
		} else {
			if (userNotFound)
				strerr_warn4(oldUser->s, "@", real_domain, ": No such user", 0);
			else
				strerr_warn1("renameuser: error connecting to db", 0);
			return (-1);
		}
		if (!islocalif(mailstore)) {
			strerr_warn6(oldUser->s, "@", real_domain, " not local (mailstore ", mailstore, ")", 0);
			return (-1);
		}
	}
#endif
	if ((pw = sql_getpw(newUser->s, real_domain))) {
		strerr_warn5("User ", newUser->s, "@", real_domain, " exists", 0);
		return (-1);
	} else
	if (!(pw = sql_getpw(oldUser->s, real_domain))) {
		if (userNotFound)
			strerr_warn4(oldUser->s, "@", real_domain, ": No such user", 0);
		else
			strerr_warn1("renameuser: Error connecting to db", 0);
		return (-1);
	} else {
		if (!stralloc_copys(&oldDir, pw->pw_dir) ||
				!stralloc_0(&oldDir))
			die_nomem();
	}
	inactive_flag = is_inactive;
	if (!(real_domain = get_real_domain(newDomain->s))) {
		strerr_warn3("Domain ", newDomain->s, " does not exist", 0);
		return (-1);
	} else
	if (!get_assign(real_domain, 0, 0, 0)) {
		strerr_warn3("Domain ", real_domain, " does not exist", 0);
		return (-1);
	}

	ptr = (char *) NULL;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &enc_pass);
		if (i != 6 && i != 8)
			strerr_die1x(1, "renameuser: unable to get secrets");
		i = str_rchr(pw->pw_passwd, ',');
		if (pw->pw_passwd[i]) {
			if (!stralloc_copyb(&scram, pw->pw_passwd, i) ||
					!stralloc_0(&scram))
				die_nomem();
			scram.len--;
			ptr = scram.s;
		}
		pw->pw_passwd = enc_pass;
	} else
	if (!str_diffn(pw->pw_passwd, "{CRAM}", 6)) {
		i = str_rchr(pw->pw_passwd, ',');
		if (pw->pw_passwd[i]) {
			if (!stralloc_copyb(&scram, pw->pw_passwd, i) ||
					!stralloc_0(&scram))
				die_nomem();
			scram.len--;
			ptr = scram.s;
			pw->pw_passwd += (i + 1);
		} else
			ptr = pw->pw_passwd;
	}
	if ((err = iadduser(newUser->s, newDomain->s, 0, pw->pw_passwd, pw->pw_gecos,
		pw->pw_shell, 0, !inactive_flag, 0, ptr)) == -1)
		return (-1);
	else
	if (!(pw = sql_getpw(newUser->s, newDomain->s))) {
		if (userNotFound)
			strerr_warn4(newUser->s, "@", newDomain->s, ": No such user", 0);
		else
			strerr_warn1("renameuser: Error connecting to db", 0);
		return (-1);
	}
	create_flag = !access(oldDir.s, F_OK);
	if (create_flag && MoveFile(oldDir.s, pw->pw_dir)) {
		strerr_warn5("renameuser: MoveFile: ", oldDir.s, " --> ", pw->pw_dir, ": ", &strerr_sys);
		return (-1);
	}
	/*
	 * TODO
	 * rename dot qmail files
	 */

#ifdef ENABLE_AUTH_LOGGING
	if (!stralloc_copyb(&SqlBuf, "update low_priority ignore lastauth set user=\"", 46) ||
			!stralloc_cat(&SqlBuf, newUser) ||
			!stralloc_catb(&SqlBuf, "\", domain=\"", 11) ||
			!stralloc_cat(&SqlBuf, newDomain) ||
			!stralloc_catb(&SqlBuf, "\" where user=\"", 14) ||
			!stralloc_cat(&SqlBuf, oldUser) ||
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
			strerr_warn4("renameuser: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if (inactive_flag)
		return (deluser(oldUser->s, real_domain, 1));
#endif
#ifdef VALIAS
	if (!stralloc_copyb(&SqlBuf, "update low_priority valias set alias=\"", 38) ||
			!stralloc_cat(&SqlBuf, newUser) ||
			!stralloc_catb(&SqlBuf, "\", domain=\"", 11) ||
			!stralloc_cat(&SqlBuf, newDomain) ||
			!stralloc_catb(&SqlBuf, "\" where alias=\"", 15) ||
			!stralloc_cat(&SqlBuf, oldUser) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
		} else {
			strerr_warn4("renameuser: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	/*- Replace all occurence of OldDir with NewDir */
	if (!stralloc_copy(&oldEmail, oldUser) ||
			!stralloc_append(&oldEmail, "@") ||
			!stralloc_cat(&oldEmail, oldDomain) ||
			!stralloc_0(&oldEmail))
		die_nomem();
	if (!stralloc_copy(&newEmail, newUser) ||
			!stralloc_append(&newEmail, "@") ||
			!stralloc_cat(&newEmail, newDomain) ||
			!stralloc_0(&newEmail))
		die_nomem();
	for(;;) {
		tmp_domain.len = 0;
		if (!(ptr = valias_select_all(&User, &tmp_domain)))
			break;
		if ((i = replacestr(ptr, oldEmail.s, newEmail.s, &tmpbuf)) == -1)
			continue;
		if (!i)
			continue;
		valias_update(User.s, tmp_domain.s, ptr, tmpbuf.s);
	}
#endif
#ifdef VFILTER
	if (!stralloc_copyb(&SqlBuf, "update low_priority vfilter set emailid=\"", 41) ||
			!stralloc_cat(&SqlBuf, newUser) ||
			!stralloc_append(&SqlBuf, "@") ||
			!stralloc_cat(&SqlBuf, newDomain) ||
			!stralloc_catb(&SqlBuf, "\" where emailid=\"", 17) ||
			!stralloc_cat(&SqlBuf, oldUser) ||
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
			strerr_warn4("renameuser: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
#endif
#ifdef ENABLE_AUTH_LOGGING
	if (!stralloc_catb(&SqlBuf, "update low_priority userquota set user=\"", 40) ||
			!stralloc_cat(&SqlBuf, newUser) ||
			!stralloc_catb(&SqlBuf, "\", domain=\"", 11) ||
			!stralloc_cat(&SqlBuf, newDomain) ||
			!stralloc_catb(&SqlBuf, "\" where user=\"", 14) ||
			!stralloc_cat(&SqlBuf, oldUser) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_cats(&SqlBuf, real_domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
				return (-1);
		} else {
			strerr_warn4("renameuser: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
#endif
	if (deluser(oldUser->s, real_domain, 1))
		return (-1);
	return (0);
}

/*
 * $Log: renameuser.c,v $
 * Revision 1.9  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.8  2023-07-15 00:19:31+05:30  Cprogrammer
 * copy scram field from old user to new user when renaming user
 *
 * Revision 1.7  2023-03-23 22:35:13+05:30  Cprogrammer
 * ignore duplicate error when updating lastauth table
 *
 * Revision 1.6  2022-11-02 14:56:35+05:30  Cprogrammer
 * restore scram password while renaming user
 *
 * Revision 1.5  2022-09-14 08:47:36+05:30  Cprogrammer
 * extract encrypted password from pw->pw_passwd starting with {SCRAM-SHA.*}
 *
 * Revision 1.4  2022-08-05 22:57:27+05:30  Cprogrammer
 * removed apop argument to iadduser()
 *
 * Revision 1.3  2022-08-05 21:14:38+05:30  Cprogrammer
 * added encrypt_flag argument to iadduser()
 *
 * Revision 1.2  2021-09-12 20:17:53+05:30  Cprogrammer
 * moved replacestr to libqmail
 *
 * Revision 1.1  2019-04-15 12:36:45+05:30  Cprogrammer
 * Initial revision
 *
 */
