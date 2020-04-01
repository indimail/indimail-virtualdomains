/*
 * $Log: vsetpass.c,v $
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
#define _XOPEN_SOURCE
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
static char     sccsid[] = "$Id: vsetpass.c,v 1.3 2020-04-01 18:59:14+05:30 Cprogrammer Exp mbhangui $";
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
	out("vsetpass", "454-");
	out("vchkpass", str);
	out("vchkpass", ": ");
	out("vsetpass", error_str(errno));
	out("vsetpass", " (#4.3.0)\r\n");
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
	char            strnum[FMT_ULONG];
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
			out("vsetpass", "454-failed to connect to database (");
			out("vsetpass", error_str(errno));
			out("vsetpass", ") (#4.3.0)\r\n");
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
		out("vsetpass", "454-failed to connect to database (");
		out("vsetpass", error_str(errno));
		out("vsetpass", ") (#4.3.0)\r\n");
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
	crypt_pass = pw->pw_passwd;
	if (env_get("DEBUG")) {
		strerr_warn11("vsetpass: login [", login, "] old_pass [",
				old_pass, "] new_pass [", new_pass, "] response [", response, "] pw_passwd [", crypt_pass, "]\n", 0);
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) (*response ? old_pass : 0),
		(unsigned char *) (*response ? response : old_pass), 0))
	{
		pipe_exec(argv, authstr, offset);
		print_error("exec");
		_exit (111);
	}
	mkpasswd(new_pass, &Crypted, encrypt_flag);
	if (env_get("DEBUG")) {
		strerr_warn11("vsetpass: login [", login, "] old_pass [",
				old_pass, "] new_pass [", new_pass, "] response [", response, "] pw_passwd [", Crypted.s, "]\n", 0);
	}
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
