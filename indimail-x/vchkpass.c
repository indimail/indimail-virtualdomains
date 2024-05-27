/*
 * $Id: vchkpass.c,v 1.21 2024-05-27 22:53:42+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef ENABLE_DOMAIN_LIMITS
#include <time.h>
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
#include <subfd.h>
#include <getEnvConfig.h>
#include <authmethods.h>
#include <get_scram_secrets.h>
#endif
#include "parse_email.h"
#include "sqlOpen_user.h"
#include "sql_getpw.h"
#include "vlimits.h"
#include "common.h"
#include "iopen.h"
#include "iclose.h"
#include "inquery.h"
#include "pipe_exec.h"
#include "indimail.h"
#include "variables.h"
#include "runcmmd.h"

#ifndef lint
static char     sccsid[] = "$Id: vchkpass.c,v 1.21 2024-05-27 22:53:42+05:30 Cprogrammer Exp mbhangui $";
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
	subprintfe(subfdout, "vchkpass", "454-%s: %s (#4.3.0)\r\n", str, error_str(errno));
	flush("vchkpass");
}

static void
die_nomem()
{
	strerr_warn1("vchkpass: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *authstr, *login, *ologin, *response, *challenge, *cleartxt,
				   *crypt_pass, *ptr, *cptr, *pass;
	char            strnum[FMT_ULONG], module_pid[FMT_ULONG];
	static stralloc user = {0}, fquser = {0}, domain = {0}, buf = {0};
	int             i, count, offset, norelay = 0, status, auth_method,
					native_checkpassword, enable_cram;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits = { 0 };
#endif

	if (argc < 2)
		_exit(2);
	if (!(authstr = alloc((authlen + 1) * sizeof(char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	/*- allow pw_passwd field to be used for CRAM authentication */
	enable_cram = env_get("ENABLE_CRAM") ? 1 : 0;
	for (offset = 0;;) {
		do {
			count = read(3, authstr + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_warn1("vchkpass: read: ", &strerr_sys);
			_exit(111);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1)) /*- string greater than 512 */
			_exit(2);
	}
	/*- max permissible length is offset */
	count = 0;
	login = authstr + count; /*- username */
	for (;authstr[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset) /*- found a non-null beyond offset */
		_exit(2);

	count++;
	challenge = authstr + count; /*- challenge for CRAM methods (or plain text password for LOGIN/PLAIN) */
	for (;authstr[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	response = authstr + count; /*- response (CRAM methods, etc) */
	for (; authstr[count] && count < offset; count++);

	if (count == offset || (count + 1) == offset)
		auth_method = 0;
	else
		auth_method = authstr[count + 1];

	ologin = login;
	for (ptr = login; *ptr && *ptr != '@'; ptr++);
	if (!*ptr) { /*- no @ in the login */
		if (auth_method == AUTH_DIGEST_MD5) { /*- for handling dumb programs like
												outlook written by dumb programmers */
			if ((ptr = str_str(login, "realm="))) {
				ptr += 6;
				for (i = 0, cptr = ptr; *ptr && *ptr != ','; ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					i++;
				}
				if (!stralloc_ready(&domain, i + 1))
					die_nomem();
				for (i = 0, ptr = cptr; *ptr && *ptr != ','; ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					domain.s[i++] = *ptr;
				}
				domain.len = i;
				if (!stralloc_0(&domain))
					die_nomem();
				domain.len--;
				for (i = 0, ptr = login; *ptr && *ptr != '@'; i++, ptr++);
				if (!stralloc_copyb(&fquser, login, i) ||
						!stralloc_append(&fquser, "@") ||
						!stralloc_cat(&fquser, &domain) ||
						!stralloc_0(&fquser))
					die_nomem();
				login = fquser.s;
			}
		}
	}
	if (!env_unset("HOME"))
		die_nomem();
	native_checkpassword = (env_get("NATIVE_CHECKPASSWORD") || env_get("DOVECOT_VERSION")) ? 1 : 0;
	if (native_checkpassword) {
		if (!env_unset("userdb_uid") || !env_unset("userdb_gid") ||
				!env_unset("EXTRA"))
			die_nomem();
	}
	parse_email(login, &user, &domain);
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE")) {
#ifdef CLUSTERED_SITE
		if (sqlOpen_user(login, 0))
#else
		if (iopen((char *) 0))
#endif
		{
			if (userNotFound)
				native_checkpassword ? _exit (1) : pipe_exec(argv, authstr, offset);
			else
#ifdef CLUSTERED_SITE
				strerr_warn1("sqlOpen_user: failed to connect to db: ", &strerr_sys);
#else
				strerr_warn1("iopen: failed to connect to db: ", &strerr_sys);
#endif
			subprintfe(subfdout, "vchkpass", "454-failed to connect to database: %s (#4.3.0)\r\n", error_str(errno));
			flush("vchkpass");
			_exit (111);
		}
		pw = sql_getpw(user.s, domain.s);
		iclose();
		ptr = "sql_getpw";
	} else {
		pw = inquery(PWD_QUERY, login, 0);
		ptr = "inquery";
	}
#else
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(login, 0))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound)
			native_checkpassword ? _exit (1) : pipe_exec(argv, authstr, offset);
		else
#ifdef CLUSTERED_SITE
			strerr_warn1("sqlOpen_user: failed to connect to db: ", &strerr_sys);
#else
			strerr_warn1("iopen: failed to connect to db: ", &strerr_sys);
#endif
		subprintfe(subfdout, "vchkpass", "454-failed to connect to database (%s) (#4.3.0)\r\n", error_str(errno));
		flush("vchkpass");
		_exit (111);
	}
	pw = sql_getpw(user.s, domain.s);
	ptr = "sql_getpw";
	iclose();
#endif
	if (!pw) {
		if (userNotFound)
			native_checkpassword ? _exit (1) : pipe_exec(argv, authstr, offset);
		else
			strerr_warn3("vchkpass: ", ptr, ": ", &strerr_sys);
		print_error(ptr);
		_exit (111);
	} else
	if (pw->pw_gid & NO_SMTP) {
		out("vchkpass", "553-Sorry, this account cannot use SMTP (#5.7.1)\r\n");
		flush("vchkpass");
		_exit (1);
	} else
	if (is_inactive && !env_get("ALLOW_INACTIVE")) {
		out("vchkpass", "553-Sorry, this account is inactive (#5.7.1)\r\n");
		flush("vchkpass");
		_exit (1);
	} else
	if (pw->pw_gid & NO_RELAY)
		norelay = 1;
	crypt_pass = (char *) NULL;
		/*- we have three situations
		 * 1. We have set SCRAM method in (-m option in vadddomain, vadduser, vpasswd,
		 *    vmoduser, vgroup)
		 *    a. For CRAM methods we use the clear text passwords in variable cleartxt
		 *    b. For SCRAM methods, we can use stored key/server key or hex salted passwords
		 *    c. For LOGIN, PLAIN we use the crypt_pass in variable crypt_pass
		 * 2. We don't have SCRAM passwords
		 *    a. we use crypted password in variable crypt_pass for LOGIN, PLAIN.
		 *       we expect a crypted password using crypt(3) is stored in pw_password field
		 *    b. we use cleartxt password in variable crypt_pass for CRAM methods (-e option in
		 *       vadddomain, vadduser, vpasswd, vmoduser, vgroup)
		 *       but it is expected that the pw_passwd field has the clear text passwords stored for
		 *       CRAM to succeed.
		 * 3. 2b implies that the crypted password can be supplied by the client for password
		 *       when using CRAM. We allow this if enable_cram is set by setting ENABLE_CRAM
		 */
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, &cleartxt, &crypt_pass);
		if (i != 6 && i != 8) {
			out("vchkpass", "553-Sorry, unable to get secrets (#5.7.1)\r\n");
			flush("vchkpass");
			_exit (1);
		}
		if (i == 8) { /* both cleartxt and hexsalted passwords are stored in db */
			switch (auth_method)
			{
			case AUTH_CRAM_MD5:
			case AUTH_CRAM_SHA1:
			case AUTH_CRAM_SHA224:
			case AUTH_CRAM_SHA256:
			case AUTH_CRAM_SHA384:
			case AUTH_CRAM_SHA512:
			case AUTH_CRAM_RIPEMD:
			case AUTH_DIGEST_MD5:
				pass = cleartxt; /*- use the clear text password stored in db. */
				break;
			default:
				if (enable_cram)
					pass = crypt_pass;
				else
					pass = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : crypt_pass;
				break;
			}
		} else {
			if (enable_cram)
				pass = crypt_pass;
			else
				pass = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : crypt_pass;
		}
	} else
	if (!str_diffn(pw->pw_passwd, "{CRAM}", 6)) {
		pw->pw_passwd += 6;
		cleartxt = pw->pw_passwd;
		i = str_rchr(pw->pw_passwd, ',');
		if (pw->pw_passwd[i]) {
			pw->pw_passwd[i] = 0;
			pw->pw_passwd += (i + 1);
		}
		switch (auth_method)
		{
		case AUTH_CRAM_MD5:
		case AUTH_CRAM_SHA1:
		case AUTH_CRAM_SHA224:
		case AUTH_CRAM_SHA256:
		case AUTH_CRAM_SHA384:
		case AUTH_CRAM_SHA512:
		case AUTH_CRAM_RIPEMD:
		case AUTH_DIGEST_MD5:
			pass = cleartxt;
			break;
		default:
			pass = crypt_pass = pw->pw_passwd;
			break;
		}
	} else {
		i = 0;
		crypt_pass = pw->pw_passwd;
		if (enable_cram)
			pass = crypt_pass;
		else
			pass = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : crypt_pass;
	}
	module_pid[fmt_ulong(module_pid, getpid())] = 0;
	if ((ptr = env_get("DEBUG_LOGIN")) && *ptr > '0') {
		ptr = (char *) get_authmethod(auth_method);
		strerr_warn16("vchkpass: ", "pid [", module_pid, "]: login [", login,
				"] challenge [", challenge, "] response [", response,
				"] password [", pass ? pass : "cram-disabled", "] crypted [",
				crypt_pass, "] authmethod [", ptr, "]", 0);
	} else
	if (env_get("DEBUG")) {
		ptr = (char *) get_authmethod(auth_method);
		strerr_warn8("vchkpass: ", "pid [", module_pid, "]: login [", login,
				"] authmethod [", ptr, "]", 0);
	}
	if (!pass) {
		native_checkpassword ? _exit (1) : pipe_exec(argv, authstr, offset);
		print_error("exec");
		_exit (111);
	}
	if (!pass || !*pass || pw_comp((unsigned char *) ologin, (unsigned char *) pass,
				(unsigned char *) (*response ? challenge : 0),
				(unsigned char *) (*response ? response : challenge), auth_method)) {
		native_checkpassword ? _exit (1) : pipe_exec(argv, authstr, offset);
		print_error("exec");
		_exit (111);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!env_get("QUERY_CACHE")) {
			if (vget_limits(domain.s, &limits)) {
				strerr_warn2("vchkpass: unable to get domain limits for for ", domain.s, 0);
				subprintfe(subfdout, "vchkpass", "454-unable to get domain limits for %s (#4.3.0)\r\n", domain.s);
				flush("vchkpass");
				_exit (111);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(domain.s, &limits)) {
			strerr_warn2("vchkpass: unable to get domain limits for for ", domain.s, 0);
			subprintfe(subfdout, "vchkpass", "454-unable to get domain limits for %s (#4.3.0)\r\n", domain.s);
			flush("vchkpass");
			_exit (111);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry) {
			out("vchkpass", "553-Sorry, your domain has expired (#5.7.1)\r\n");
			flush("vchkpass");
			_exit (1);
		} else
		if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry) {
			out("vchkpass", "553-Sorry, your password has expired (#5.7.1)\r\n");
			flush("vchkpass");
			_exit (1);
		}
	}
#endif
	status = 0;
	if (!env_put2("HOME", pw->pw_dir))
		die_nomem();
	if ((ptr = (char *) env_get("POSTAUTH")) && !access(ptr, X_OK)) {
		if (!stralloc_copys(&buf, ptr) ||
				!stralloc_append(&buf, " ") ||
				!stralloc_cats(&buf, login) ||
				!stralloc_0(&buf))
			die_nomem();
		status = runcmmd(buf.s, 0);
	}
	if (native_checkpassword) { /*- support dovecot checkpassword */
		if (!env_put2("userdb_uid", "indimail") ||
				!env_put2("userdb_gid", "indimail"))
			die_nomem();
		if ((ptr = env_get("EXTRA"))) {
			if (!stralloc_copyb(&buf, "userdb_uid userdb_gid ", 22) ||
					!stralloc_cats(&buf, ptr) || !stralloc_0(&buf))
				die_nomem();
		} else
		if (!stralloc_copyb(&buf, "userdb_uid userdb_gid", 21) ||
				!stralloc_0(&buf))
			die_nomem();
		if (!env_put2("EXTRA", buf.s))
			die_nomem();
		execv(argv[1], argv + 1);
		print_error("exec");
		_exit (111);
	}
	_exit(norelay ? 3 : status);
	/*- Not reached */
	return(0);
}

/*
 * $Log: vchkpass.c,v $
 * Revision 1.21  2024-05-27 22:53:42+05:30  Cprogrammer
 * initialize struct vlimits
 *
 * Revision 1.20  2024-05-10 11:43:51+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.19  2023-07-15 12:50:01+05:30  Cprogrammer
 * authenticate using CRAM when password field starts with {CRAM}
 *
 * Revision 1.18  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.17  2022-11-05 21:13:57+05:30  Cprogrammer
 * use ENABLE_CRAM to allow use of pw_passwd field of indimail, indibak for authentication
 *
 * Revision 1.16  2022-10-29 23:10:52+05:30  Cprogrammer
 * fixed display of auth method in logs
 *
 * Revision 1.15  2022-08-27 12:04:41+05:30  Cprogrammer
 * fixed logic for fetching clear txt password for cram methods
 *
 * Revision 1.14  2022-08-25 18:03:04+05:30  Cprogrammer
 * fetch clear text passwords for CRAM authentication
 *
 * Revision 1.13  2022-08-23 08:21:44+05:30  Cprogrammer
 * display AUTH method as a string instead of a number
 *
 * Revision 1.12  2022-08-04 14:43:02+05:30  Cprogrammer
 * authenticate using SCRAM salted password
 *
 * Revision 1.11  2021-09-11 13:41:27+05:30  Cprogrammer
 * fixed typo in error statement
 *
 * Revision 1.10  2021-07-22 15:17:34+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.9  2021-01-27 18:46:10+05:30  Cprogrammer
 * renamed use_dovecot to native_checkpassword
 *
 * Revision 1.8  2021-01-27 13:23:25+05:30  Cprogrammer
 * use use_dovecot variable instead of env_get() twice
 *
 * Revision 1.7  2021-01-26 14:17:22+05:30  Cprogrammer
 * set HOME, userdb_uid, userdb_gid, EXTRA env variables for dovecot
 *
 * Revision 1.6  2021-01-26 13:45:03+05:30  Cprogrammer
 * modified to support dovecot checkpassword authentication
 *
 * Revision 1.5  2020-09-28 13:28:28+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.4  2020-09-28 12:49:53+05:30  Cprogrammer
 * print authmodule name in error logs/debug statements
 *
 * Revision 1.3  2020-04-01 18:58:32+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-07-10 12:58:04+05:30  Cprogrammer
 * print more error information in print_error
 *
 * Revision 1.1  2019-04-18 08:14:23+05:30  Cprogrammer
 * Initial revision
 *
 */
