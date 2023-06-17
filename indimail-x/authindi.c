/*
 * $Log: authindi.c,v $
 * Revision 1.17  2023-06-17 23:46:55+05:30  Cprogrammer
 * set PASSWORD_HASH to make pw_comp use crypt() instead of in_crypt()
 *
 * Revision 1.16  2023-03-21 09:32:51+05:30  Cprogrammer
 * replaced strerr_warn with subprintfe
 *
 * Revision 1.15  2022-09-10 21:41:56+05:30  Cprogrammer
 * use authmethods.h for AUTH type definitions
 * execute next module if domain not in qmail assign file
 *
 * Revision 1.14  2022-08-27 12:05:20+05:30  Cprogrammer
 * fixed logic for fetching clear txt password for cram methods
 *
 * Revision 1.13  2022-08-25 18:02:20+05:30  Cprogrammer
 * fetch clear text passwords for CRAM authentication
 *
 * Revision 1.12  2022-08-04 14:37:42+05:30  Cprogrammer
 * authenticate using SCRAM salted password
 *
 * Revision 1.11  2021-01-19 22:45:13+05:30  Cprogrammer
 * added uid, gid in debug
 *
 * Revision 1.10  2020-10-13 18:30:57+05:30  Cprogrammer
 * use _exit(2) as no buffers need to be flushed
 *
 * Revision 1.9  2020-10-04 09:27:08+05:30  Cprogrammer
 * use AUTHADDR to determine if we are already authenticated
 *
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
#include <authmethods.h>
#include <get_scram_secrets.h>
#include <noreturn.h>
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
#include "common.h"

#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#else
#define AUTH_SIZE 512
#endif

#define FATAL "authindi: fatal: "
#define WARN  "authindi: warn: "

#ifndef lint
static char     sccsid[] = "$Id: authindi.c,v 1.17 2023-06-17 23:46:55+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc tmpbuf = {0};
static int      authlen = AUTH_SIZE;
static char     strnum[FMT_ULONG];

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
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

/*- This executes imapd, pop3. */
no_return static void
failure1(char **argv, int auth_method, char *service,
		char *imapargs[], char *pop3args[], char *authstr, char *challenge)
{
	if (write(2, "AUTHFAILURE\n", 12) == -1)
		;
	close_connection();
	if (auth_method > AUTH_PLAIN) {
		if (challenge) {
			alloc_free(challenge);
			challenge = 0;
		}
		alloc_free(authstr);
	}
	execv(!str_diff("pop3", service) ? *pop3args : *imapargs, argv);
	strerr_die4x(111, FATAL, "execv ", !str_diff("pop3", service) ? *pop3args : *imapargs, ": ");
}

/*- This executes next auth module. */
no_return static void
next_module(char **argv, char *buf, int offset, int auth_method, char *authstr, char *challenge)
{
	close_connection();
	if (auth_method > AUTH_PLAIN) {
		if (challenge) {
			alloc_free(challenge);
			challenge = 0;
		}
		alloc_free(authstr);
	}
	pipe_exec(argv, buf, offset);
	_exit(111);
}

no_return void
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
		close_connection();
		strerr_die3x(100, WARN, "Login not permitted for ", service);
	}
	close_connection();
	if (!env_put2("AUTHENTICATED", userid))
		die_nomem();
	if (!stralloc_copy(&tmpbuf, &TheUser) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, TheDomain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("AUTHADDR", tmpbuf.s))
		die_nomem();
	if (!env_put2("AUTHFULLNAME", pw->pw_gecos))
		die_nomem();
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
		strerr_die3x(100, "unable to parse_quota [", pw->pw_shell, "]");
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
	if (!env_put2("MAILDIRQUOTA", tmpbuf.s) ||
			!env_put2("HOME", pw->pw_dir) ||
			!env_put2("AUTHSERVICE", service))
		die_nomem();
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
						strerr_warn4(FATAL, "open: ", ptr, ": ", &strerr_sys);
					else {
						substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
						for (;;) {
							if (getln(&ssin, &line, &match, '\n') == -1) {
								strerr_warn4(FATAL, "read: ", ptr, ": ", &strerr_sys);
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
				strerr_die2x(100, FATAL, "POSTAUTH: Error on Exit");
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
	if (chdir(Maildir.s))
		strerr_die4sys(111, FATAL, "chdir: ", Maildir.s, ": ");
	if (!stralloc_copy(&tmpbuf, &Maildir) ||
			!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("MAILDIR", tmpbuf.s))
		die_nomem();
	execv(argv[0], argv);
	strerr_die4sys(111, FATAL, "exec: ", argv[1], ": ");
}

int
main(int argc, char **argv)
{
	char           *buf, *authstr, *login, *challenge, *response, *cleartxt, *crypt_pass, *ptr,
				   *real_domain, *service, *auth_type, *auth_data, *mailstore, *pass;
	char           *(imapargs[]) = { PREFIX"/sbin/imaplogin", LIBEXECDIR"/imapmodules/authindi",
					PREFIX"/bin/imapd", "Maildir", 0 };
	char           *(pop3args[]) = { PREFIX"/sbin/pop3login", LIBEXECDIR"/imapmodules/authindi",
					PREFIX"/bin/pop3d", "Maildir", 0 };
	static stralloc user = {0}, domain = {0}, Email = {0};
	int             i, count, offset, auth_method, debug;
	size_t          cram_md5_len, out_len;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if (argc < 2)
		_exit (2);
	debug = env_get("DEBUG") ? 1 : 0;
	ptr = env_get("AUTHADDR");
	if (ptr && *ptr) {
		execv(*(argv + argc - 2), argv + argc - 2);
		strerr_die4sys(111, FATAL, "execv: ", *(argv + argc - 2), ": ");
	}
	i = str_rchr(argv[0], '/');
	if (argc < 3)
		strerr_die2x(100, FATAL, "no more modules will be tried");
	if (debug) {
		subprintfe(subfderr, "authindi", "debug: authindi: uid[%u] euid[%u] %s %s\n",
				getuid(), geteuid(), argc > 3 ? "program" : "module", argv[1]);
		substdio_flush(subfderr);
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
		do {
			count = read(3, authstr + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			alloc_free(authstr);
			strerr_die2x(111, FATAL, "read: ");
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1)) {
			alloc_free(authstr);
			strerr_die2x(2, FATAL, "auth data too long");
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
		alloc_free(buf);
		alloc_free(authstr);
		strerr_die2x(2, FATAL, "auth data too short");
	}
	authstr[count++] = 0;

	auth_type = authstr + count; /* type (login, plain, cram-md5, cram-sha1 or pass) */
	for (;authstr[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset) {
		alloc_free(buf);
		alloc_free(authstr);
		strerr_die2x(2, FATAL, "auth data too short");
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
	for (cram_md5_len = 0; authstr[count] != '\n' && count < offset; count++, cram_md5_len++);
	if (count == offset || (count + 1) == offset) {
		alloc_free(buf);
		alloc_free(authstr);
		strerr_die2x(2, FATAL, ": auth data too short");
	}
	authstr[count++] = 0;
	if (auth_method > AUTH_PLAIN) {
		if (!(ptr = b64_decode((unsigned char *) login, cram_md5_len, &out_len))) {
			strerr_warn2(FATAL, "b64_decode failure", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, 0);
		}
		challenge = ptr;
	} else
		challenge = 0;

	auth_data = authstr + count; /*- (plain text password or cram-md5 response) */
	for (cram_md5_len = 0;authstr[count] != '\n' && count < offset;count++, cram_md5_len++);
	authstr[count++] = 0;
	if (auth_method > AUTH_PLAIN) {
		if (!(ptr = b64_decode((unsigned char *) auth_data, cram_md5_len, &out_len))) {
			strerr_warn2(FATAL, "b64_decode failure", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		for (login = ptr; *ptr && !isspace(*ptr); ptr++);
		*ptr = 0;
		response = ptr + 1;
	} else
		response = 0;
	if (!*auth_data) {
		auth_data = authstr + count; /* in case of auth login, auth plain */
		for (;authstr[count] != '\n' && count < offset;count++);
		authstr[count++] = 0;
	}
	if (!str_diffn(auth_type, "pass", 5)) {
		strerr_warn2(FATAL, "Password Change not supported", 0);
		next_module(argv, buf, offset, auth_method, login, challenge);
	}
	parse_email(login, &user, &domain);
	if (!get_assign(domain.s, 0, &uid, &gid)) {
		strerr_warn4(FATAL, "domain ", domain.s, " does not exist", 0);
		next_module(argv, buf, offset, auth_method, login, challenge);
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
		strerr_warn3(FATAL, real_domain, ": is_distributed_domain failed", 0);
		failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
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
				strerr_warn3(FATAL, "No mailstore for ", Email.s, 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		i = str_rchr(mailstore, ':');
		if (mailstore[i])
			mailstore[i] = 0;
		else {
			strerr_warn5(FATAL, "invalid mailstore [", mailstore, "] for ", Email.s, 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		for(; *mailstore && *mailstore != ':'; mailstore++);
		if  (*mailstore != ':') {
			strerr_warn5(FATAL, "invalid mailstore [", mailstore, "] for ", Email.s, 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		mailstore++;
		if (!islocalif(mailstore)) {
			strerr_warn5(FATAL, Email.s, " not on local (mailstore ", mailstore, ")", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
	}
#endif /*- CLUSTERED_SITE */
#ifdef QUERY_CACHE
	if (env_get("QUERY_CACHE"))
		pw = inquery(PWD_QUERY, Email.s, 0);
	else {
		if (iopen((char *) 0)) {
			strerr_warn2(FATAL, "failed to connect to local db", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		pw = sql_getpw(user.s, real_domain);
	}
#else
	if (iopen((char *) 0)) {
		strerr_warn2(FATAL, "failed to connect to local db", 0);
		failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
	}
	pw = sql_getpw(user.s, real_domain);
#endif
	if (!pw) {
		if(!userNotFound)
#ifdef QUERY_CACHE
			strerr_warn5(FATAL, Email.s, ": ",
					env_get("QUERY_CACHE") ? "inquery: " : "iopen: ",
					errno ? error_str(errno) : "AUTHFAILURE", 0);
#else
			strerr_warn4(FATAL, Email.s, ": iopen: ",
					errno ? error_str(errno) : "AUTHFAILURE", 0);
#endif
		failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (str_diff("webmail", service) == 0) {
		if (pw->pw_gid & NO_WEBMAIL) {
			strerr_warn2(WARN, "webmail disabled for this account", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
	} else
	if (str_diff("pop3", service) == 0) {
		if (pw->pw_gid & NO_POP) {
			strerr_warn2(WARN, "pop3 disabled for this account", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
	} else
	if (str_diff("imap", service) == 0) {
		if (pw->pw_gid & NO_IMAP) {
			strerr_warn2(WARN, "imap disabled for this account", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
	}
	crypt_pass = (char *) NULL;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, &cleartxt, &crypt_pass);
		if (i != 6 && i != 8) {
			strerr_warn2(FATAL, "unable to get secrets", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		if (i == 8) {
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
				pass = crypt_pass;
				break;
			}
		} else
			pass = crypt_pass;
	} else {
		i = 0;
		pass = crypt_pass = pw->pw_passwd;
	}
	if ((ptr = env_get("DEBUG_LOGIN")) && *ptr > '0') {
		subprintfe(subfderr, "authindi",
				"debug_login: pid[%d] service[%s] login[%s] password[%s] crypted[%s] authmethod[%s]",
				getpid(), service, login, auth_data, crypt_pass, auth_type);
		if (challenge)
			subprintfe(subfderr, "authindi", " challenge[%s]", challenge);
		if (response)
			subprintfe(subfderr, "authindi", " response[%s]", response);
		subprintfe(subfderr, "authindi", "\n");
		substdio_flush(subfderr);
	} else
	if (debug) {
		subprintfe(subfderr, "authindi", "debug: authindi: pid [%d] service[%s]: login[%s] authmethod [%s]\n",
			getpid(), service, login, auth_type);
		substdio_flush(subfderr);
	}
	/*- force pw_comp to use crypt instead of in_crypt */
	if (!env_get("PASSWORD_HASH") && !env_put2("PASSWORD_HASH", "0"))
		die_nomem();
	if (pw_comp((unsigned char *) login, (unsigned char *) pass,
		(unsigned char *) (auth_method > AUTH_PLAIN ? challenge : 0),
		(unsigned char *) (auth_method > AUTH_PLAIN ? response : auth_data), auth_method)) {
		if (argc == 3) {
			strerr_warn2(FATAL, "no more modules will be tried", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, challenge);
		}
		next_module(argv, buf, offset, auth_method, authstr, challenge);
	}
	if (auth_method > AUTH_PLAIN) {
		if (challenge) {
			alloc_free(challenge);
			challenge = 0;
		}
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!env_get("QUERY_CACHE")) {
			if (vget_limits(real_domain, &limits)) {
				strerr_warn3(FATAL, "unable to get domain limits for for ", real_domain, 0);
				failure1(argv, auth_method, service, imapargs, pop3args, authstr, 0);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(real_domain, &limits)) {
			strerr_warn3(FATAL, "unable to get domain limits for for ", real_domain, 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, 0);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry) {
			strerr_warn2(FATAL, "Sorry, your domain has expired", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, 0);
		} else
		if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry) {
			strerr_warn2(FATAL, "Sorry, your password has expired", 0);
			failure1(argv, auth_method, service, imapargs, pop3args, authstr, 0);
		}
	}
#endif
	alloc_free(authstr);
	exec_local(argv + argc - 2, login, real_domain, pw, service);
}
