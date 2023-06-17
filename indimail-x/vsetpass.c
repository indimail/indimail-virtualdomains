/*
 * $Log: vsetpass.c,v $
 * Revision 1.10  2023-06-17 23:48:02+05:30  Cprogrammer
 * set PASSWORD_HASH to make pw_comp use crypt() instead of in_crypt()
 *
 * Revision 1.9  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.8  2022-09-14 08:48:10+05:30  Cprogrammer
 * extract encrypted password from pw->pw_passwd starting with {SCRAM-SHA.*}
 *
 * Revision 1.7  2022-08-05 21:23:42+05:30  Cprogrammer
 * reversed encrypt_flag setting for mkpasswd() change in encrypt_flag
 *
 * Revision 1.6  2021-07-22 15:17:42+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.5  2020-09-28 13:28:36+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.4  2020-09-28 12:50:12+05:30  Cprogrammer
 * removed extra newline
 *
 * Revision 1.3  2020-04-01 18:59:14+05:30  Cprogrammer
 * added encrypt flag argument to mkpasswd()
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-07-10 12:58:10+05:30  Cprogrammer
 * print more error information in print_error
 *
 * Revision 1.1  2019-04-18 08:37:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#include <env.h>
#include <error.h>
#include <pw_comp.h>
#include <mkpasswd.h>
#include <getEnvConfig.h>
#include <get_scram_secrets.h>
#include <subfd.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#endif
#include "sqlOpen_user.h"
#include "iopen.h"
#include "pipe_exec.h"
#include "variables.h"
#include "sql_getpw.h"
#include "inquery.h"
#include "iclose.h"
#include "check_quota.h"
#include "sql_passwd.h"
#include "vset_lastauth.h"
#include "common.h"
#include "parse_email.h"
#include "getpeer.h"

#ifndef lint
static char     sccsid[] = "$Id: vsetpass.c,v 1.10 2023-06-17 23:48:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#else
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;

void
print_error(char *str)
{
	subprintfe(subfdout, "vsetpass", "454-%s: %s (#4.3.0)\r\n", str, error_str(errno));
	flush("vsetpass");
}

static void
die_nomem()
{
	strerr_warn1("vsetpass: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *authstr, *login, *new_pass, *old_pass, *response, *crypt_pass, *ptr;
	char            strnum[FMT_ULONG], module_pid[FMT_ULONG];
	static stralloc Dir = {0}, Crypted = {0}, user = {0}, domain = {0};
	int             count, offset, i;
	mdir_t          quota;
	struct passwd  *pw;

	if (argc < 2)
		_exit(2);
	if (!(authstr = alloc((authlen + 1) * sizeof(char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	for (offset = 0;;) {
		do
		{
			count = read(3, authstr + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_warn1("syspass: read: ", &strerr_sys);
			_exit(111);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = authstr + count; /*- username */
	for (;authstr[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	old_pass = authstr + count; /*- challenge (or plain text) */
	for (;authstr[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	response = authstr + count; /*- response */
	for (;authstr[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	new_pass = authstr + count; /*- new password */
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE")) {
#ifdef CLUSTERED_SITE
		if (sqlOpen_user(login, 0))
#else
		if (iopen((char *) 0))
#endif
		{
			if (userNotFound)
				pipe_exec(argv, authstr, offset);
			else
#ifdef CLUSTERED_SITE
				strerr_warn1("sqlOpen_user: failed to connect to db: ", &strerr_sys);
#else
				strerr_warn1("iopen: failed to connect to db: ", &strerr_sys);
#endif
			subprintfe(subfdout, "vsetpass", "454-failed to connect to database: %s (#4.3.0)\r\n", error_str(errno));
			flush("vsetpass");
			_exit (111);
		}
	}
#else
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(login, 0))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound)
			pipe_exec(argv, authstr, offset);
		else
#ifdef CLUSTERED_SITE
			strerr_warn1("sqlOpen_user: failed to connect to db: ", &strerr_sys);
#else
			strerr_warn1("iopen: failed to connect to db: ", &strerr_sys);
#endif
		subprintfe(subfdout, "vsetpass", "454-failed to connect to database: %s (#4.3.0)\r\n", error_str(errno));
		flush("vsetpass");
		_exit (111);
	}
#endif
	parse_email(login, &user, &domain);
#ifdef QUERY_CACHE
	if (env_get("QUERY_CACHE")) {
		pw = inquery(PWD_QUERY, login, 0);
		ptr = "inquery";
	} else {
		pw = sql_getpw(user.s, domain.s);
		ptr = "sql_getpw";
	}
#else
	pw = sql_getpw(user.s, domain.s);
	ptr = "sql_getpw";
#endif
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE"))
		iclose();
#else /*- Not QUERY_CACHE */
	iclose();
#endif
	if (!pw) {
		if (userNotFound)
			pipe_exec(argv, authstr, offset);
		else
			strerr_warn3("vsetpass: ", ptr, ": ", &strerr_sys);
		print_error(ptr);
		_exit (111);
	}
	if (pw->pw_gid & NO_PASSWD_CHNG) {
		out("vsetpass", "553 Sorry, this account cannot change password (#5.7.1)\r\n");
		flush("vsetpass");
		_exit (1);
	} else
	if (is_inactive && !env_get("ALLOW_INACTIVE")) {
		out("vsetpass", "553 Sorry, this account has expired (#5.7.1)\r\n");
		flush("vsetpass");
		_exit (1);
	}
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	crypt_pass = (char *) NULL;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &crypt_pass);
		if (i != 6 && i != 8) {
			out("vsetpass", "454 unable to get secrets (#4.3.0)\r\n");
			flush("vsetpass");
			strerr_die1(1, "vsetpass: unable to get secrets", 0);
		}
	} else {
		i = 0;
		crypt_pass = pw->pw_passwd;
	}
#else
	crypt_pass = pw->pw_passwd;
#endif
#else
	crypt_pass = pw->pw_passwd;
#endif
	module_pid[fmt_ulong(module_pid, getpid())] = 0;
	if (env_get("DEBUG_LOGIN"))
		strerr_warn13("vsetpass: pid [", module_pid, "] login [", login, "] old_pass [",
				old_pass, "] new_pass [", new_pass, "] response [", response, "] pw_passwd [", crypt_pass, "]", 0);
	if (!env_get("PASSWORD_HASH") && !env_put2("PASSWORD_HASH", "0"))
		die_nomem();
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) (*response ? old_pass : 0),
		(unsigned char *) (*response ? response : old_pass), 0))
	{
		pipe_exec(argv, authstr, offset);
		print_error("exec");
		_exit (111);
	}
	mkpasswd(new_pass, &Crypted, 1);
	if (env_get("DEBUG_LOGIN"))
		strerr_warn11("vsetpass: login [", login, "] old_pass [",
				old_pass, "] new_pass [", new_pass, "] response [", response, "] pw_passwd [", Crypted.s, "]", 0);
	if ((i = sql_passwd(user.s, domain.s, Crypted.s, 0)) == 1) {
		if (!stralloc_copys(&Dir, pw->pw_dir) ||
				!stralloc_catb(&Dir, "/Maildir", 8) ||
				!stralloc_0(&Dir))
			die_nomem();
		if(access(Dir.s, F_OK))
			quota = 0l;
		else {
#ifdef USE_MAILDIRQUOTA
			quota = check_quota(Dir.s, 0);
#else
			quota = check_quota(Dir.s);
#endif
		}
#ifdef ENABLE_AUTH_LOGGING
		if (!(ptr = GetPeerIPaddr())) {
			strerr_warn1("vsetpass: GetPeerIPaddr: ", &strerr_sys);
			return (-1);
		}
		vset_lastauth(user.s, domain.s, "pass", ptr, pw->pw_gecos, quota);
#endif
	}
	return(i == 1 ? 0 : 1);
}
