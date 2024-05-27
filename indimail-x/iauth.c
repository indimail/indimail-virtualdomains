/*
 * $Id: iauth.c,v 1.11 2024-05-27 22:51:20+05:30 Cprogrammer Exp mbhangui $
 *
 * authenticate.c - Generic PAM Authentication module for pam_multi
 * Copyright (C) <2008-2023>  Manvendra Bhangui <manvendra@indimail.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The GNU General Public License does not permit incorporating your program
 * into proprietary programs.  If your program is a subroutine library, you
 * may consider it more useful to permit linking proprietary applications with
 * the library.  If this is what you want to do, use the GNU Lesser General
 * Public License instead of this License.  But first, please read
 * <http://www.gnu.org/philosophy/why-not-lgpl.html>.
 *
 * Testing
 * pamtester imap postmaster@indimail.org authenticate
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#undef PACKAGE
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef ENABLE_AUTH_LOGGING
#include <mysql.h>
#include <mysqld_error.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <case.h>
#include <env.h>
#include <strerr.h>
#include <byte.h>
#include <fmt.h>
#include <scan.h>
#include <getEnvConfig.h>
#include <get_scram_secrets.h>
#endif
#include "sqlOpen_user.h"
#include "iopen.h"
#include "iclose.h"
#include "inquery.h"
#include "vlimits.h"
#include "get_assign.h"
#include "create_table.h"
#include "get_real_domain.h"
#include "sql_getpw.h"
#include "parse_email.h"
#include "limits.h"
#include "variables.h"
#include "parse_quota.h"
#include "Login_Tasks.h"
#include "Check_Login.h"
#include "common.h"

#ifndef lint
static char     sccsid[] = "$Id: iauth.c,v 1.11 2024-05-27 22:51:20+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      defaultTask(char *, char *, struct passwd *, char *, int);

static void
close_connection()
{
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE"))
		iclose();
#else /*- Not QUERY_CACHE */
	iclose();
#endif
}

struct passwd  *_global_pw;

static char *
i_auth(const char *email, const char *service, int *size, int debug)
{
	static stralloc User = {0}, Domain = {0};
	const char     *real_domain;
	char           *crypt_pass;
	char            strnum[FMT_ULONG];
	int             i;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;

	_global_pw = (struct passwd *) 0;
	if (size)
		*size = 0;
	if (parse_email(email, &User, &Domain)) {
		strerr_warn1("iauth.so: out of memory", 0);
		return ((char *) 0);
	}
	User.len--;
	Domain.len--;
	/*- crypt("pass", "kk"); -*/
	if (debug)
		strerr_warn1("iauth.so: opening MySQL connection", 0);
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE")) {
#ifdef CLUSTERED_SITE
		if (sqlOpen_user(email, 0))
#else
		if (iopen((char *) 0))
#endif
			return ((char *) 0);
	}
#else
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 0))
#else
	if (iopen((char *) 0))
#endif
		return ((char *) 0);
#endif
#ifdef QUERY_CACHE
	if (env_get("QUERY_CACHE")) {
		if (debug)
			strerr_warn1("iauth.so: doing inquery", 0);
		pw = inquery(PWD_QUERY, email, 0);
	} else {
		if (debug)
			strerr_warn1("iauth.so: doing sql_getpw", 0);
		if (!get_assign(Domain.s, 0, &uid, &gid)) {
			strerr_warn3("iauth.so: domain ", Domain.s, " does not exist", 0);
			return ((char *) 0);
		}
		if (!(real_domain = get_real_domain(Domain.s)))
			real_domain = Domain.s;
		pw = sql_getpw(User.s, real_domain);
	}
#else
	if (debug)
		strerr_warn1("iauth.so: doing sql_getpw", 0);
	if (!get_assign(Domain.s, 0, &uid, &gid)) {
		strerr_warn3("iauth.so: domain ", Domain.s, " does not exist", 0);
		return ((char *) 0);
	}
	if (!(real_domain = get_real_domain(Domain.s)))
		real_domain = Domain.s;
	pw = sql_getpw(User.s, real_domain);
#endif
	if (!pw) {
		if(userNotFound)
			return ((char *) 0);
		else
			strerr_warn1("i_auth: inquery: ", &strerr_sys);
		close_connection();
		return ((char *) 0);
	}
	crypt_pass = (char *) NULL;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &crypt_pass);
		if (i != 6 && i != 8) {
			close_connection();
			strerr_warn1("i_auth: unable to get secrets", 0);
		}
	} else
	if (!str_diffn(pw->pw_passwd, "{CRAM}", 6)) {
		i = str_rchr(pw->pw_passwd, ',');
		if (pw->pw_passwd[i])
			pw->pw_passwd += (i + 1);
		else
			pw->pw_passwd += 6;
		crypt_pass = pw->pw_passwd;
	} else
		crypt_pass = pw->pw_passwd;
	if (env_get("DEBUG_LOGIN"))
		strerr_warn7("i_auth: service[", service ? service : "null", "] email [", email, "] pw_passwd [", crypt_pass, "]", 0);
	close_connection();
	_global_pw = pw;
	if (size) {
		*size = str_len(crypt_pass) + 1;
		if (debug) {
			strnum[fmt_ulong(strnum, *size)] = 0;
			strerr_warn2("iauth.so: returning data of size ", strnum, 0);
		}
	}
	return (crypt_pass);
}

#ifdef ENABLE_AUTH_LOGGING
#define NO_OF_ITEMS 2
char           *
i_acctmgmt(char *email, char *service, int *size, int *nitems, int debug)
{
	static stralloc User = {0}, Domain = {0}, SqlBuf = {0};
	char           *ptr;
	const char     *real_domain;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             i, exp_day;
	static long     exp_times[NO_OF_ITEMS];
	time_t          tmval;
	uid_t           uid;
	gid_t           gid;
	MYSQL_RES      *res;
	MYSQL_ROW       row;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits = { 0 };
#endif

	if (nitems)
		*nitems = NO_OF_ITEMS;
	if (size)
		*size = 0;
	parse_email(email, &User, &Domain);
	if (debug)
		strerr_warn1("iauth.so: i_acctmgmt: opening MySQL connection", 0);
	if (iopen((char *) 0))
		return ((char *) 0);
	if (!get_assign(Domain.s, 0, &uid, &gid)) {
		strerr_warn3("iauth.so: i_acctmgmt: domain ", Domain.s, " does not exist", 0);
		return ((char *) 0);
	}
	if (!(real_domain = get_real_domain(Domain.s)))
		real_domain = Domain.s;
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!env_get("QUERY_CACHE")) {
			if (vget_limits(real_domain, &limits)) {
				strerr_warn2("iauth.so: i_acctmgmt: unable to get domain limits for for ", real_domain, 0);
				out("iauth.so", "454-unable to get domain limits for ");
				out("iauth.so", real_domain);
				out("iauth.so", "\r\n");
				flush("iauth.so");
				_exit (111);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, email, 0);
#else
		if (vget_limits(real_domain, &limits)) {
			strerr_warn2("iauth.so: i_acctmgmt: unable to get domain limits for for ", real_domain, 0);
			return ((char *) 0);
		}
		lmt = &limits;
#endif
		exp_times[0] = lmt->domain_expiry == -1 ? 0 : lmt->domain_expiry;
		exp_times[1] = lmt->passwd_expiry == -1 ? 0 : lmt->passwd_expiry;
		if (debug) {
			strnum1[fmt_ulong(strnum1, exp_times[0])] = 0;
			strnum2[fmt_ulong(strnum2, exp_times[1])] = 0;
			strerr_warn4("iauth.so: i_acctmgmt: expiry[0] = ", strnum1, ", expiry[1] = ", strnum2, 0);
		}
	}  else
#endif
	for (i = 0;i < NO_OF_ITEMS;i++) {
		switch (i)
		{
		case 0:
			if (!stralloc_copyb(&SqlBuf,
						"select high_priority UNIX_TIMESTAMP(timestamp) from lastauth where user=\"", 73) ||
					!stralloc_cat(&SqlBuf, &User) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cat(&SqlBuf, &Domain) ||
					!stralloc_catb(&SqlBuf, "\" and (service=\"pop3\" or service=\"imap\" or service=\"webm\")", 58) ||
					!stralloc_0(&SqlBuf))
				return ((char *) 0);
			if (!(ptr = env_get("ACCT_INACT_EXPIRY")))
				exp_day = 60;
			else
				scan_uint(ptr, (unsigned int *) &exp_day);
			break;
		case 1:
			if (!stralloc_copyb(&SqlBuf,
						"select high_priority UNIX_TIMESTAMP(timestamp) from lastauth where user=\"", 73) ||
					!stralloc_cat(&SqlBuf, &User) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cat(&SqlBuf, &Domain) ||
					!stralloc_catb(&SqlBuf, "\" and service=\"pass\"", 20) ||
					!stralloc_0(&SqlBuf))
				return ((char *) 0);
			if (!(ptr = env_get("PASSWORD_EXPIRY")))
				exp_day = 60;
			else
				scan_uint(ptr, (unsigned int *) &exp_day);
			break;
		}
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if(in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
				exp_times[i] = 0;
				continue;
			}
			strerr_warn4("iauth.so: i_acctmgmt: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((char *) 0);
		}
		res = in_mysql_store_result(&mysql[1]);
		exp_times[i] = 0;
		while ((row = in_mysql_fetch_row(res))) {
			scan_ulong(row[0], (unsigned long *) &tmval);
			if (tmval > exp_times[i])
				exp_times[i] = tmval;
		}
		if (exp_times[i])
			exp_times[i] += exp_day * 86400;
		if (debug) {
			strnum1[fmt_uint(strnum1, i)] = 0;
			strnum2[fmt_ulong(strnum2, exp_times[i])] = 0;
			strerr_warn4("iauth.so: i_acctmgmt: expiry[", strnum1, "] = ", strnum2, 0);
		}
		in_mysql_free_result(res);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (debug)
		strerr_warn4("service=[", service ? service : "null", "] pw_name = ",
			_global_pw ? _global_pw->pw_name : "absent", 0);
	if (service && _global_pw) {
		if (!byte_diff("webm", 4, service)) {
			if (_global_pw->pw_gid & NO_WEBMAIL) {
				strerr_warn1("iauth.so: i_acctmgmt: webmail disabled for this account", 0);
				close_connection();
				return ((char *) 0);
			}
		} else
		if (!byte_diff("pop3", 4, service)) {
			if (_global_pw->pw_gid & NO_POP) {
				strerr_warn1("iauth.so: i_acctmgmt: pop3 disabled for this account", 0);
				close_connection();
				return ((char *) 0);
			}
		} else
		if (!byte_diff("imap", 4, service)) {
			if (_global_pw->pw_gid & NO_IMAP) {
				strerr_warn1("iauth.so: i_acctmgmt: imap disabled for this account", 0);
				close_connection();
				return ((char *) 0);
			}
		}
		if (defaultTask(email, Domain.s, _global_pw, service, debug)) {
			close_connection();
			return ((char *) 0);
		}
	}
	if (nitems) {
		if (size)
			*size = sizeof(long) * NO_OF_ITEMS;
		return ((char *) &exp_times[0]);
	} else {
		if (size)
			*size = 0;
		return ((char *) 0);
	}
}
#else
char           *
i_acctmgmt(char *email, char *service, int *size, int *nitems, int debug)
{
	*nitems = 0;
	return ((char *) 0);
}
#endif

char *
iauth(char *email, char *service, int auth_or_accmgmt, int *size, int *nitems, int debug)
{
	if (!auth_or_accmgmt && nitems)
		*nitems = 1;
	return (auth_or_accmgmt ?  i_acctmgmt(email, service, size, nitems, debug) : i_auth(email, service, size, debug));
}

static int
defaultTask(char *email, char *TheDomain, struct passwd *pw, char *service, int debug)
{
	static stralloc Maildir = {0}, TheUser = {0}, tmpbuf = {0};
	char            strnum[FMT_ULONG];
	char           *ptr;
	int             status, len;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif

	if (parse_email(email, &TheUser, &tmpbuf))
		return (1);
	for (len = 0, ptr = service; *ptr && *ptr != ':'; ptr++, len++);
	if (!stralloc_copyb(&tmpbuf, service, len) || !stralloc_0(&tmpbuf))
		return (1);
	if (debug)
		strerr_warn1("iauth.so: defaultTask: checking Login permission", 0);
	if (Check_Login(tmpbuf.s, TheDomain, pw->pw_gecos)) {
		strerr_warn2("Login not permitted for ", tmpbuf.s, 0);
		return (1);
	}
	if (debug)
		strerr_warn1("iauth.so: defaultTask: performing Login Tasks", 0);
	status = Login_Tasks(pw, email, tmpbuf.s);
	if (status == 2 && !case_diffb(service, 4, "imap"))
		return (1);
	if (!stralloc_copys(&Maildir, status == 2 ? "/mail/tmp" : pw->pw_dir) ||
			!stralloc_catb(&Maildir, "/Maildir", 8) ||
			!stralloc_0(&Maildir))
		return (1);
	if (access(pw->pw_dir, F_OK) || access(Maildir.s, F_OK) || chdir(pw->pw_dir)) {
		strerr_warn2("iauth.so: defaultTask: chdir: ", pw->pw_dir, &strerr_sys);
		return (1);
	}
	if (!env_put2("AUTHENTICATED", email))
		return (1);
	if (!stralloc_copys(&tmpbuf, TheUser.s) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, TheDomain) ||
			!stralloc_0(&tmpbuf))
		return (1);
	if (!env_put2("AUTHADDR", tmpbuf.s))
		return (1);
	if (!env_put2("AUTHADDR", pw->pw_gecos))
		return (1);
	if (debug)
		strerr_warn1("iauth.so: defaultTask: checking quota", 0);
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1) {
		strerr_warn3("auth.so: defaultTask: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copyb(&tmpbuf, strnum, fmt_ulong(strnum, size_limit)) ||
			!stralloc_catb(&tmpbuf, "S,", 2) ||
			!stralloc_catb(&tmpbuf, strnum, fmt_ulong(strnum, count_limit)) ||
			!stralloc_append(&tmpbuf, "C") ||
			!stralloc_0(&tmpbuf))
		return (1);
#else
	if (!stralloc_copys(&tmpbuf, str_diffn(pw->pw_shell, "NOQUOTA", 8) ? pw->pw_shell : "0") ||
			!stralloc_append(&tmpbuf, "S") ||
			!stralloc_0(&tmpbuf))
		return (1);
#endif
	if (!env_put2("MAILDIRQUOTA", tmpbuf.s))
		return (1);
	if (!env_put2("HOME", pw->pw_dir))
		return (1);
	if (!env_put2("AUTHSERVICE", service))
		return (1);
	if (!env_put2("MAILDIR", Maildir.s))
		return (1);
	return (0);
}

/*
 * $Log: iauth.c,v $
 * Revision 1.11  2024-05-27 22:51:20+05:30  Cprogrammer
 * initialize struct vlimits
 *
 * Revision 1.10  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2023-07-15 00:17:48+05:30  Cprogrammer
 * authenticate using CRAM when password field starts with {CRAM}
 *
 * Revision 1.8  2022-10-29 21:36:14+05:30  Cprogrammer
 * removed compiler warning for unused variable
 *
 * Revision 1.7  2022-09-13 22:04:09+05:30  Cprogrammer
 * extract encrypted password from pw->pw_passwd starting with {SCRAM-SHA.*}
 *
 * Revision 1.6  2020-10-04 08:34:13+05:30  Cprogrammer
 * allow size paramter to be null
 *
 * Revision 1.5  2020-09-23 10:56:17+05:30  Cprogrammer
 * avoid potential SIGSEGV if nitiems is 0
 *
 * Revision 1.4  2020-04-01 18:55:14+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-04-22 23:11:22+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.2  2019-04-15 10:23:30+05:30  Cprogrammer
 * removed strnum2 variable
 *
 * Revision 1.1  2019-04-15 10:08:29+05:30  Cprogrammer
 * Initial revision
 */
