/*
 * $Log: authindi.c,v $
 * Revision 1.8  2020-10-01 18:19:57+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.7  2020-09-29 20:51:27+05:30  Cprogrammer
 * execute next module when already authenticated by previous module
 *
 * Revision 1.6  2020-09-29 11:13:18+05:30  Cprogrammer
 * fixed module name to 'authindi'
 *
 * Revision 1.5  2020-09-28 13:28:00+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.4  2020-09-28 12:48:11+05:30  Cprogrammer
 * print authmodule name in error logs/debug statements
 *
 * Revision 1.3  2020-06-03 17:06:16+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.2  2020-04-01 18:52:49+05:30  Cprogrammer
 * moved pw_comp.h to libqmail
 *
 * Revision 1.1  2019-04-17 02:31:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <byte.h>
#include <error.h>
#include <str.h>
#include <env.h>
#include <fmt.h>
#include <open.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <pw_comp.h>
#endif
#include "parse_email.h"
#include "get_assign.h"
#include "get_real_domain.h"
#include "is_distributed_domain.h"
#include "inquery.h"
#include "findhost.h"
#include "islocalif.h"
#include "iopen.h"
#include "iclose.h"
#include "variables.h"
#include "vlimits.h"
#include "Check_Login.h"
#include "Login_Tasks.h"
#include "parse_quota.h"
#include "pipe_exec.h"
#include "sql_getpw.h"

#ifndef lint
static char     sccsid[] = "$Id: authindi.c,v 1.8 2020-10-01 18:19:57+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#else
#define AUTH_SIZE 512
#endif

static int      exec_local(char **, char *, char *, struct passwd *, char *);

static stralloc tmpbuf = {0};
static int      authlen = AUTH_SIZE;
static char     strnum[FMT_ULONG], module_pid[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("authindi: out of memory", 0);
	_exit(111);
}

void
close_connection()
{
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE"))
		iclose();
#else /*- Not QUERY_CACHE */
	iclose();
#endif
}

static unsigned char encoding_table[] = { 
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static char    *decoding_table = NULL;

void
build_decoding_table()
{
	int i;

	decoding_table = (char *) alloc(256);
	for (i = 0; i < 0x40; i++)
		decoding_table[encoding_table[i]] = i;
}

static char    *
b64_decode(const unsigned char *data, size_t input_length, size_t *output_length)
{
	int i, j;

	if (decoding_table == NULL)
		build_decoding_table();

	if (input_length % 4 != 0) {
		errno = 0;
		return NULL;
	}

	*output_length = input_length / 4 * 3;
	if (data[input_length - 1] == '=')
		(*output_length)--;
	if (data[input_length - 2] == '=')
		(*output_length)--;

	char           *decoded_data = (char *) alloc(*output_length);
	if (decoded_data == NULL)
		return NULL;

	for (i = 0, j = 0; i < input_length;) {

		uint32_t        sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

		uint32_t        triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < *output_length)
			decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}
	return decoded_data;
}

void
base64_cleanup()
{
	alloc_free((char *) decoding_table);
}

int
main(int argc, char **argv)
{
	char           *buf, *authstr, *login, *challenge, *response, *crypt_pass, *ptr,
				   *real_domain, *prog_name, *service, *auth_type, *auth_data, *mailstore;
	char           *(imapargs[]) = { PREFIX"/sbin/imaplogin", LIBEXECDIR"/imapmodules/authindi",
					PREFIX"/bin/imapd", "Maildir", 0 };
	char           *(pop3args[]) = { PREFIX"/sbin/pop3login", LIBEXECDIR"/imapmodules/authindi",
					PREFIX"/bin/pop3d", "Maildir", 0 };
	static stralloc user = {0}, domain = {0}, Email = {0};
	int             i, count, offset, auth_method;
	size_t          cram_md5_len, out_len;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if (argc < 2)
		exit (2);
	if ((ptr = env_get("AUTHENTICATED"))) {
		execv(argv[1], argv + 1);
		strerr_warn3("execv: ", argv[1], ": ", &strerr_sys);
	}
	i = str_rchr(argv[0], '/');
	if (argv[0][i])
		prog_name = argv[0] + i + 1;
	else
		prog_name = argv[0];
	if (argc < 3) {
		strerr_warn2(prog_name, ": no more modules will be tried", 0);
		return (1);
	}
	if (!(authstr = alloc((authlen + 1) * sizeof(char)))) {
		if (write(2, "AUTHFAILURE\n", 12) == -1)
			;
		die_nomem();
	}
	/*-
	 * Courier-IMAP authmodules Protocol (authindi /var/indimail/bin/imapd Maildir < /tmp/input 3<&0)
	 * imap\n                ---> imap service (imap/pop3/pam-multi)
	 * login\n
	 * postmaster@test.com\n ---> username or challenge
	 * pass\n                ---> plain text / response
	 * newpass\n             ---> auth_data
	 * argv[0]=/usr/libexec/indimail/imapmodules/authindi
	 * argv[1]=/usr/libexec/indimail/imapmodules/authpam
	 * argv[2]=/usr/bin/imapd
	 * argv[3]=Maildir
	 */
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
			strerr_warn1("authindi: read: ", &strerr_sys);
			return (1);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1)) {
			strerr_warn2(prog_name, ": auth data too long", 0);
			return (2);
		}
	}
	if (!(buf = alloc((offset + 1) * sizeof(char)))) {
		if (write(2, "AUTHFAILURE\n", 12) == -1)
			;
		die_nomem();
	}
	byte_copy(buf, offset, authstr);
	count = 0;
	service = authstr + count; /*- service */
	for (;authstr[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset) {
		strerr_warn2(prog_name, ": auth data too short", 0);
		return (2);
	}
	authstr[count++] = 0;

	auth_type = authstr + count; /* type (login, plain, cram-md5, cram-sha1 or pass) */
	for (;authstr[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset) {
		strerr_warn2(prog_name, ": auth data too short", 0);
		return (2);
	}
	authstr[count++] = 0;
	if (!str_diffn(auth_type, "pass", 5))
		auth_method = -1;
	else
	if (!str_diffn(auth_type, "login", 6))
		auth_method = AUTH_LOGIN;
	else
	if (!str_diffn(auth_type, "plain", 6))
		auth_method = AUTH_PLAIN;
	else
	if (!str_diffn(auth_type, "cram-md5", 9))
		auth_method = AUTH_CRAM_MD5;
	else
	if (!str_diffn(auth_type, "cram-sha1", 10))
		auth_method = AUTH_CRAM_SHA1;
	else
	if (!str_diffn(auth_type, "cram-sha256", 12))
		auth_method = AUTH_CRAM_SHA256;
	else
	if (!str_diffn(auth_type, "cram-sha512", 12))
		auth_method = AUTH_CRAM_SHA512;
	else
	if (!str_diffn(auth_type, "cram-ripemd", 12))
		auth_method = AUTH_CRAM_RIPEMD;
	else
		auth_method = 0;

	login = authstr + count; /*- username or challenge */
	for (cram_md5_len = 0;authstr[count] != '\n' && count < offset;count++, cram_md5_len++);
	if (count == offset || (count + 1) == offset) {
		strerr_warn2(prog_name, ": auth data too short", 0);
		return (2);
	}
	authstr[count++] = 0;
	if (auth_method > 2) {
		if (!(ptr = b64_decode((unsigned char *) login, cram_md5_len, &out_len))) {
			strerr_warn1("authindi: b64_decode failure", 0);
			pipe_exec(argv, buf, offset);
			return (1);
		}
		challenge = ptr;
	} else
		challenge = 0;

	auth_data = authstr + count; /*- (plain text password or cram-md5 response) */
	for (cram_md5_len = 0;authstr[count] != '\n' && count < offset;count++, cram_md5_len++);
	authstr[count++] = 0;
	if (auth_method > 2) {
		if (!(ptr = b64_decode((unsigned char *) auth_data, cram_md5_len, &out_len))) {
			if (challenge)
				alloc_free (challenge);
			strerr_warn1("authindi: b64_decode failure", 0);
			pipe_exec(argv, buf, offset);
			return (1);
		}
		for (login = ptr;*ptr && !isspace(*ptr);ptr++);
		*ptr = 0;
		response = ptr + 1;
	} else
		response = 0;
	if (!*auth_data) {
		auth_data = authstr + count; /* in case of auth login, auth plain */
		for (;authstr[count] != '\n' && count < offset;count++);
		authstr[count++] = 0;
	}
	if (!str_diffn(auth_type, "pass", 5))
	{
		strerr_warn2(prog_name, ": Password Change not supported", 0);
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	parse_email(login, &user, &domain);
	if (!get_assign(domain.s, 0, &uid, &gid)) {
		strerr_warn4(prog_name, ": domain ", domain.s, " does not exist", 0);
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	if (!stralloc_copy(&Email, &user) ||
			!stralloc_append(&Email, "@") ||
			!stralloc_cats(&Email, real_domain) ||
			!stralloc_0(&Email))
		die_nomem();
#ifdef CLUSTERED_SITE
	if ((count = is_distributed_domain(real_domain)) == -1) {
		strerr_warn2(real_domain, ": is_distributed_domain failed", 0);
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	} else
	if (count) {
#ifdef QUERY_CACHE
		if (env_get("QUERY_CACHE"))
			mailstore = inquery(HOST_QUERY, Email.s, 0);
		else
			mailstore = findhost(Email.s, 2);
#else
		mailstore = findhost(Email.s, 2);
#endif
		if (!mailstore) {
			if (!userNotFound)
				strerr_warn2("No mailstore for ", Email.s, 0);
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
		i = str_rchr(mailstore, ':');
		if (mailstore[i])
			mailstore[i] = 0;
		else {
			strerr_warn4("authindi: invalid mailstore [", mailstore, "] for ", Email.s, 0);
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
		for(; *mailstore && *mailstore != ':'; mailstore++);
		if  (*mailstore != ':') {
			strerr_warn4("authindi: invalid mailstore [", mailstore, "] for ", Email.s, 0);
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
		mailstore++;
		if (!islocalif(mailstore)) {
			strerr_warn4(Email.s, " not on local (mailstore ", mailstore, ")", 0);
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
	}
#endif /*- CLUSTERED_SITE */
#ifdef QUERY_CACHE
	if (env_get("QUERY_CACHE"))
		pw = inquery(PWD_QUERY, Email.s, 0);
	else {
		if (iopen((char *) 0)) {
			strerr_warn1("failed to connect to local db", 0);
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			pipe_exec(argv, buf, offset);
		}
		pw = sql_getpw(user.s, real_domain);
	}
#else
	if (iopen((char *) 0)) {
		strerr_warn1("failed to connect to local db", 0);
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
	}
	pw = sql_getpw(user.s, real_domain);
#endif
	if (!pw) {
		if(!userNotFound)
#ifdef QUERY_CACHE
			strerr_warn6(prog_name, ": ", Email.s, ": ",
					env_get("QUERY_CACHE") ? "inquery: " : "iopen: ",
					errno ? error_str(errno) : "AUTHFAILURE", 0);
#else
			strerr_warn5(prog_name, ": ", Email.s, ": iopen: ",
					errno ? error_str(errno) : "AUTHFAILURE", 0);
#endif
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
		close_connection();
		return (1);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (str_diff("webmail", service) == 0) {
		if (pw->pw_gid & NO_WEBMAIL) {
			strerr_warn2(prog_name, ": webmail disabled for this account", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
			return (1);
		}
	} else
	if (str_diff("pop3", service) == 0) {
		if (pw->pw_gid & NO_POP) {
			strerr_warn2(prog_name, ": pop3 disabled for this account", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
			return (1);
		}
	} else
	if (str_diff("imap", service) == 0) {
		if (pw->pw_gid & NO_IMAP) {
			strerr_warn2(prog_name, ": imap disabled for this account", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			if (auth_method > 2) {
				if (challenge)
					alloc_free (challenge);
				alloc_free (login);
			}
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
			return (1);
		}
	}
	crypt_pass = pw->pw_passwd;
	strnum[fmt_uint(strnum, (unsigned int) auth_method)] = 0;
	module_pid[fmt_ulong(module_pid, getpid())] = 0;
	if ((ptr = env_get("DEBUG_LOGIN")) && *ptr > '0') {
		if (response)
			strerr_warn16(prog_name, ": pid [", module_pid, "] service[", service, "] authmeth [",
				strnum, "] login [", login, "] challenge [",
				challenge, "] response [", response, "] pw_passwd [", crypt_pass, "]", 0);
		else 
			strerr_warn14(prog_name, ": pid [", module_pid, "] service[", service, "] authmeth [", strnum, "] login [",
				login, "] auth [", auth_data, "] pw_passwd [", crypt_pass, "]", 0);
	} else
	if (env_get("DEBUG"))
		strerr_warn10(prog_name, ": pid [", module_pid, "] service[", service,
				"] authmeth [", strnum, "] login [", login, "]", 0);
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) (auth_method > 2 ? challenge : 0),
		(unsigned char *) (auth_method > 2 ? response : auth_data), auth_method))
	{
		if (argc == 3) {
			strerr_warn2(prog_name, ": no more modules will be tried", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
			return (1);
		}
		close_connection();
		if (auth_method > 2) {
			if (challenge)
				alloc_free (challenge);
			alloc_free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (auth_method > 2) {
		if (challenge)
			alloc_free (challenge);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!env_get("QUERY_CACHE")) {
			if (vget_limits(real_domain, &limits)) {
				strerr_warn3(prog_name, ": unable to get domain limits for for ", real_domain, 0);
				close_connection();
				if (auth_method > 2)
					alloc_free (login);
				pipe_exec(argv, buf, offset);
				return (1);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(real_domain, &limits)) {
			strerr_warn3(prog_name, ": unable to get domain limits for for ", real_domain, 0);
			close_connection();
			if (auth_method > 2)
				alloc_free (login);
			pipe_exec(argv, buf, offset);
			return (1);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry) {
			strerr_warn2(prog_name, ": Sorry, your domain has expired", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			if (auth_method > 2)
				alloc_free (login);
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
			return (1);
		} else
		if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry) {
			strerr_warn2(prog_name, ": Sorry, your password has expired", 0);
			if (write(2, "AUTHFAILURE\n", 12) == -1)
				;
			close_connection();
			if (auth_method > 2)
				alloc_free (login);
			execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
			strerr_warn4(prog_name, ": execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ", &strerr_sys);
		} 
	}
#endif
	exec_local(argv + argc - 2, login, real_domain, pw, service);
	if (auth_method > 2)
		alloc_free (login);
	return (0);
}

static int
exec_local(char **argv, char *userid, char *TheDomain, struct passwd *pw, char *service)
{
	static stralloc Maildir = {0}, TheUser = {0}, line = {0};
	char           *ptr;
	int             i, status, fd, match;
	char            inbuf[4096];
	struct substdio ssin;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif

	for (i = 0, ptr = userid; *ptr && *ptr != '@'; i++, ptr++);
	if (!stralloc_copyb(&TheUser, userid, i) ||
			!stralloc_0(&TheUser))
		die_nomem();
	TheUser.len--;
	if (Check_Login(service, TheDomain, pw->pw_gecos)) {
		strerr_warn2("Login not permitted for ", service, 0);
		close_connection();
		return (1);
	}
	if (!env_put2("AUTHENTICATED", userid))
		strerr_die3sys(111, "authindi: env_put2: AUTHENTICATED=", userid, ": ");
	if (!stralloc_copy(&tmpbuf, &TheUser) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, TheDomain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("AUTHADDR", tmpbuf.s))
		strerr_die3sys(111, "authindi: env_put2: AUTHADDR=", tmpbuf.s, ": ");
	if (!env_put2("AUTHFULLNAME", pw->pw_gecos))
		strerr_die3sys(111, "authindi: env_put2: AUTHFULLNAME=", pw->pw_gecos, ": ");
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1) {
		strerr_warn3("parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
		close_connection();
		return (1);
	}
	if (!stralloc_copyb(&tmpbuf, strnum, fmt_ulong(strnum, (unsigned long) size_limit)) ||
			!stralloc_catb(&tmpbuf, "S,", 2) ||
			!stralloc_catb(&tmpbuf, strnum, fmt_ulong(strnum, (unsigned long) count_limit)) ||
			!stralloc_append(&tmpbuf, "C") ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#else
	if (!stralloc_copys(&tmpbuf, pw->pw_shell) ||
			!stralloc_append(&tmpbuf, "S") ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#endif
	if (!env_put2("MAILDIRQUOTA", tmpbuf.s))
		strerr_die3sys(111, "authindi: env_put2: MAILDIRQUOTA=", tmpbuf.s, ": ");
	if (!env_put2("HOME", pw->pw_dir))
		strerr_die3sys(111, "authindi: env_put2: HOME=", pw->pw_dir, ": ");
	if (!env_put2("AUTHSERVICE", service))
		strerr_die3sys(111, "authindi: env_put2: AUTHENTICATED=", service, ": ");
	switch ((status = Login_Tasks(pw, userid, service)))
	{
		case 2:
			if (!stralloc_copys(&Maildir, (ptr = env_get("TMP_MAILDIR")) ? ptr : pw->pw_dir) ||
					!stralloc_0(&Maildir))
				die_nomem();
			Maildir.len--;
			break;
		case 3:
			if ((ptr = env_get("EXIT_ONERROR"))) {
				if ((ptr = env_get("MSG_ONERROR")) && !access(ptr, F_OK)) {
					if ((fd = open_read(ptr)) == -1)
						strerr_warn3("authindi: open: ", ptr, ": ", &strerr_sys);
					else {
						substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
						for (;;) {
							if (getln(&ssin, &line, &match, '\n') == -1) {
								strerr_warn3("authindi: read: ", ptr, ": ", &strerr_sys);
								close(fd);
								break;
							}
							if (!line.len)
								break;
							if (substdio_put(subfdoutsmall, line.s, line.len)) {
								close(fd);
								break;
							}
						}
						substdio_flush(subfdoutsmall);
						close(fd);
					}
				}
				strerr_warn1("POSTAUTH: Error on Exit", 0);
				return(1);
			}
			if (!stralloc_copys(&Maildir, (ptr = env_get("TMP_MAILDIR")) ? ptr : pw->pw_dir) ||
					!stralloc_0(&Maildir))
				die_nomem();
			Maildir.len--;
			break;
		default:
			if (!stralloc_copys(&Maildir, pw->pw_dir) || !stralloc_0(&Maildir))
				die_nomem();
			Maildir.len--;
			break;
	}
	close_connection();
	if (chdir(Maildir.s)) {
		strerr_warn3("authindi: chdir: ", Maildir.s, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copy(&tmpbuf, &Maildir) ||
			!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("MAILDIR", tmpbuf.s))
		strerr_die3sys(111, "authindi: env_put2: AUTHSERVICE=", service, ": ");
	execv(argv[0], argv);
	strerr_warn3("authindi: exec: ", argv[1], ": ", &strerr_sys);
	return (1);
}
