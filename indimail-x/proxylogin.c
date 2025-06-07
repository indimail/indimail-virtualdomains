/*
 * $Id: proxylogin.c,v 1.13 2025-06-07 18:17:31+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: proxylogin.c,v 1.13 2025-06-07 18:17:31+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <open.h>
#include <mkpasswd.h>
#include <pw_comp.h>
#include <getEnvConfig.h>
#include <get_scram_secrets.h>
#endif
#include "runcmmd.h"
#include "auth_admin.h"
#include "adminCmmd.h"
#include "inquery.h"
#include "strmsg.h"
#include "remove_quotes.h"
#include "variables.h"
#include "islocalif.h"
#include "monkey.h"
#include "common.h"
#include "Check_Login.h"
#include "parse_quota.h"
#include "parse_email.h"
#include "Login_Tasks.h"

static int      LocalLogin(char **, char *, char *, char *, char *, char *, char *);
static int      ExecIMAPD(char **, char *, char *, char *, struct passwd *, char *, char *);
static int      have_imap_starttls();
static int      have_pop3_starttls();
static int      tlsrequired();

static stralloc tmpbuf = {0};
static char     strnum[FMT_ULONG + 11];

static void
die_nomem()
{
	strerr_warn1("proxylogin: out of memory", 0);
	_exit(111);
}

int
autoAddUser(char *email, char *pass, char *service, int encrypt_flag)
{
	char           *admin_user, *admin_pass, *admin_host, *admin_port,
                   *hard_quota, *ptr, *certfile, *cafile, *crlfile;
	static stralloc cmdbuf = {0}, encrypted = {0};
	int             i, sfd, match_cn;

	if (!env_get("AUTOADDUSERS"))
		return (1);
	if (encrypt_flag) {
		if (mkpasswd(pass, &encrypted, encrypt_flag) == -1)
			strerr_die1sys(111, "crypt: ");
	} else {
		if (!stralloc_copys(&encrypted, pass) ||
				!stralloc_0(&encrypted))
			die_nomem();
		encrypted.len--;
	}
	getEnvConfigStr(&ptr, "ADDUSERCMD", PREFIX"/bin/autoadduser");
	if (!access(ptr, X_OK)) {
		if (!stralloc_copys(&cmdbuf, ptr) ||
				!stralloc_append(&cmdbuf, " ") ||
				!stralloc_cats(&cmdbuf, email) ||
				!stralloc_append(&cmdbuf, " ") ||
				!stralloc_cat(&cmdbuf, &encrypted) ||
				!stralloc_0(&cmdbuf))
			die_nomem();
		return (runcmmd(cmdbuf.s, 0));
	}
	if (!(admin_user = (char *) env_get("ADMIN_USER"))) {
		strerr_warn1("proxylogin: admin user not specified", 0);
		return (-1);
	} else
	if (!(admin_pass = (char *) env_get("ADMIN_PASS"))) {
		strerr_warn1("proxylogin: admin password not specified", 0);
		return (-1);
	} else
	if (!(admin_host = (char *) env_get("ADMIN_HOST"))) {
		strerr_warn1("proxylogin: admin host not specified", 0);
		return (-1);
	} else
	if (!(admin_port = (char *) env_get("ADMIN_PORT"))) {
		strerr_warn1("proxylogin: admin port not specified", 0);
		return (-1);
	} else
	if (!(certfile = (char *) env_get("CERTFILE"))) {
		strerr_warn1("proxylogin: client certificate not specified", 0);
		return (-1);
	}
	cafile = (char *) env_get("CAFILE");
	crlfile = (char *) env_get("CRLFILE");
	match_cn = env_get("MATCH_CN") ? 1 : 0;

	if ((sfd = auth_admin(admin_user, admin_pass, admin_host, admin_port, certfile, cafile, crlfile, match_cn)) == -1)
		return (-1);
	i = str_chr(service, ':');
	if (i > 4) {
		strerr_warn3("proxylogin: invalid service [", service, "]", 0);
		return (-1);
	}
	getEnvConfigStr(&hard_quota, "HARD_QUOTA", HARD_QUOTA);
	if (!stralloc_copyb(&cmdbuf, "0 vadduser -e -c auto.", 22) ||
			!stralloc_cats(&cmdbuf, admin_port) ||
			!stralloc_append(&cmdbuf, ".") ||
			!stralloc_catb(&cmdbuf, service, 4) ||
			!stralloc_catb(&cmdbuf, " -q ", 4) ||
			!stralloc_cats(&cmdbuf, hard_quota) ||
			!stralloc_append(&cmdbuf, " ") ||
			!stralloc_cats(&cmdbuf, email) ||
			!stralloc_append(&cmdbuf, " ") ||
			!stralloc_cat(&cmdbuf, &encrypted) ||
			!stralloc_0(&cmdbuf))
		die_nomem();
	cmdbuf.len--;
	return (adminCmmd(sfd, 0, cmdbuf.s, cmdbuf.len));
}

static int
LocalLogin(char **argv, char *email, char *TheUser, char *TheDomain, char *service,
	char *imaptag, char *plaintext)
{
	char           *p, *crypt_pass;
	int             i;
	struct passwd  *pw;

	if (!(pw = inquery(PWD_QUERY, email, 0))) {
		if (userNotFound) {
			if (!str_diffn(service, "imap", 4)) {
				if (!env_put2("IMAPLOGINTAG", imaptag))
					strerr_die3sys(111, "proxylogin: env_put2: IMAPLOGINTAG=", imaptag, ": ");
			}
			execv(argv[0], argv);
			strerr_die2sys(111, "proxylogin: execv: ", argv[0]);
		} else {
			strmsg_out1("proxylogin: inquery PWD_QUERY temporary failure\n");
			strerr_warn1("proxylogin: inquery PWD_QUERY temporary failure", 0);
		}
		return (1);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (str_diff("webmail", service) == 0) {
		if (pw->pw_gid & NO_WEBMAIL) {
			strerr_warn1("proxylogin: webmail disabled for this account", 0);
			return (-1);
		}
	} else
	if (str_diff("pop3", service) == 0) {
		if (pw->pw_gid & NO_POP) {
			strerr_warn1("proxylogin: pop3 disabled for this account", 0);
			return (-1);
		}
	} else
	if (str_diff("imap", service) == 0) {
		if (pw->pw_gid & NO_IMAP) {
			strerr_warn1("proxylogin: imap disabled for this account", 0);
			return (-1);
		}
	}
	p = plaintext;
	remove_quotes(&p);
	crypt_pass = (char *) NULL;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &crypt_pass);
		if (i != 6 && i != 8) {
			strerr_warn1("proxylogin: unable to get secrets", 0);
			return (-1);
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
	if (pw->pw_passwd[0] && !pw_comp(0, (unsigned char *) crypt_pass, 0, (unsigned char *) p, 0)) {
		if (!str_diffn(service, "imap", 4)) {
			if (!env_put2("IMAPLOGINTAG", imaptag))
				strerr_die3sys(111, "proxylogin: env_put2: IMAPLOGINTAG=", imaptag, ": ");
		}
		ExecIMAPD(argv, email, TheUser, TheDomain, pw, service, imaptag);
		strerr_warn9("proxylogin: email [", email, "] domain[", TheDomain, "] service [", service, "] imaptag [", imaptag, "]: ", &strerr_sys);
	}
	if (!str_diffn(service, "imap", 4)) {
		if (!env_put2("IMAPLOGINTAG", imaptag))
			strerr_die3sys(111, "proxylogin: env_put2: IMAPLOGINTAG=", imaptag, ": ");
	}
	execv(argv[0], argv);
	strerr_warn3("proxylogin: execv: ", argv[0], ": ", &strerr_sys);
	return (1);
}

int
proxylogin(char **argv, char *service, char *userid, char *plaintext, char *remoteip, char *imaptag, int skip_nl)
{
	char           *ptr, *email, *remote_port;
	static stralloc loginbuf = {0}, TheUser = {0}, TheDomain = {0};
	char           *mailstore;
	int             i, retval;

	email = userid;
	remove_quotes(&email);
	parse_email(email, &TheUser, &TheDomain);
	i = str_rchr(service, ':');
	if (service[i])
		remote_port = service + i + 1;
	else {
		strmsg_out3("proxylogin: unknown service [", service, "]\n");
		strerr_warn3("proxylogin: unknown service [", service, "]", 0);
		return (1);
	}
	if (str_diffn(service, "imap", 4) && str_diffn(service, "pop3", 4)) {
		strmsg_out5("proxylogin: unknown service [", service, "] remote_port [", remote_port, "]\n");
		strerr_warn5("proxylogin: unknown service [", service, "] remote_port [", remote_port, "]", 0);
		return (1);
	}
	mailstore = (char *) 0;
	if (!(mailstore = inquery(HOST_QUERY, email, 0))) {
		if (userNotFound) {
			switch ((retval = autoAddUser(email, plaintext, service, 1)))
			{
			case -1:
				strmsg_out1("proxylogin: autoAddUser failed\n");
				strerr_warn1("proxylogin: autoAddUser failed", 0);
				return (1);
			case 0: /*- user successfully provisioned */
				break;
			case 1: /*- auth failure */
				if (!str_diffn(service, "imap", 4)) {
					if (!env_put2("IMAPLOGINTAG", imaptag))
						strerr_die3sys(111, "proxylogin: env_put2: IMAPLOGINTAG=", imaptag, ": ");
				}
				execv(argv[0], argv);
				strerr_warn3("proxylogin: execv: ", argv[0], ": ", &strerr_sys);
				return (1);
			}
		} else {
			strmsg_out3("proxylogin: inquery HOST_QUERY: ", email, " temporary failure\n");
			strerr_warn3("proxylogin: inquery HOST_QUERY: ", email, " temporary failure", 0);
			return (1);
		}
	}
	/*- user provisioned by autoAdduser */
	if (!mailstore && !(mailstore = inquery(HOST_QUERY, email, 0))) {
		if (userNotFound) {
			if (!str_diffn(service, "imap", 4)) {
				if (!env_put2("IMAPLOGINTAG", imaptag))
					strerr_die3sys(111, "proxylogin: env_put2: IMAPLOGINTAG=", imaptag, ": ");
			}
			execv(argv[0], argv);
			strerr_warn3("proxylogin: execv: ", argv[0], ": ", &strerr_sys);
		} else {
			strmsg_out3("proxylogin: inquery HOST_QUERY: ", email, " temporary failure\n");
			strerr_warn3("proxylogin: inquery HOST_QUERY: ", email, " temporary failure", 0);
		}
		return (1);
	}
	i = str_rchr(mailstore, ':');
	if (mailstore[i])
		mailstore[i] = 0;
	else {
		strerr_warn4("proxylogin: invalid mailstore [", mailstore, "] for ", email, 0);
		return (1);
	}
	for(; *mailstore && *mailstore != ':'; mailstore++);
	if  (*mailstore != ':') {
		strerr_warn4("proxylogin: invalid mailstore [", mailstore, "] for ", email, 0);
		return (1);
	}
	mailstore++;
	if (islocalif(mailstore)) { /* - Prevent LoopBak */
		return (LocalLogin(argv, email, TheUser.s, TheDomain.s, service, imaptag, plaintext));
	}
	else { /*- if (islocalif(mailstore)) */
		if (!str_diffn(service, "imap", 4)) {
			if (!stralloc_copys(&loginbuf, imaptag) ||
					!stralloc_catb(&loginbuf, " LOGIN ", 7) ||
					!stralloc_cat(&loginbuf, &TheUser) ||
					!stralloc_append(&loginbuf, "@") ||
					!stralloc_cat(&loginbuf, &TheDomain))
				die_nomem();
			if (!(ptr = env_get("LEGACY_SERVER"))) {
				if (!stralloc_append(&loginbuf, ":") ||
						!stralloc_cats(&loginbuf, remoteip))
					die_nomem();
			}
			if (!stralloc_append(&loginbuf, " ") ||
					!stralloc_cats(&loginbuf, plaintext) ||
					!stralloc_catb(&loginbuf, "\r\n", 2) ||
					!stralloc_0(&loginbuf))
				die_nomem();
		}
		if (!str_diffn(service, "pop3", 4)) {
			if (!stralloc_copyb(&loginbuf, "USER ", 5) ||
					!stralloc_cat(&loginbuf, &TheUser) ||
					!stralloc_append(&loginbuf, "@") ||
					!stralloc_cat(&loginbuf, &TheDomain))
				die_nomem();
			if (!(ptr = env_get("LEGACY_SERVER"))) {
				if (!stralloc_append(&loginbuf, ":") ||
						!stralloc_cats(&loginbuf, remoteip))
					die_nomem();
			}
			if (!stralloc_catb(&loginbuf, "\nPASS ", 6) ||
					!stralloc_cats(&loginbuf, plaintext) ||
					!stralloc_append(&loginbuf, "\n") ||
					!stralloc_0(&loginbuf))
				die_nomem();
		}
		strerr_warn9("INFO: LOGIN, user=", TheUser.s, "@", TheDomain.s, ", ip=[", remoteip, "], mailstore=[", mailstore, "]", 0);
		retval = monkey(mailstore, remote_port, loginbuf.s, skip_nl);
		if (retval == 2) {
			strerr_warn9("ERR: DISCONNECTED, user=", TheUser.s, "@", TheDomain.s, ", ip=[", remoteip, "], mailstore=[", mailstore, "]", 0);
			strmsg_out1("* BYE Disconnected for inactivity.\r\n");
		} else
			strerr_warn9("INFO: LOGOUT, user=", TheUser.s, "@", TheDomain.s, ", ip=[", remoteip, "], mailstore=[", mailstore, "]", 0);
		return (retval);
	}
}

static int
ExecIMAPD(char **argv, char *userid, char *TheUser, char *TheDomain, struct passwd *pw, char *service, char *imaptag)
{
	char            strnum1[FMT_ULONG];
	static stralloc Maildir = {0}, line = {0};
	char           *ptr;
	int             i, fd, status, match;
	char            inbuf[4096];
	struct substdio ssin;
#ifdef USE_MAILDIRQUOTA
	int             j;
	char            strnum2[FMT_ULONG];
	mdir_t          size_limit, count_limit;
#endif

	if (Check_Login(service, TheDomain, pw->pw_gecos)) {
		strerr_warn2("proxylogin: login not permitted for ", service, 0);
		return (1);
	}
	if (!env_put2("AUTHENTICATED", userid))
		strerr_die3sys(111, "proxylogin: env_put2: AUTHENTICATED=", userid, ": ");
	if (!stralloc_copys(&tmpbuf, TheUser) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, TheDomain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("AUTHADDR", tmpbuf.s))
		strerr_die3sys(111, "proxylogin: env_put2: AUTHADDR=", tmpbuf.s, ": ");
	if (!env_put2("AUTHFULLNAME", pw->pw_gecos))
		strerr_die3sys(111, "proxylogin: env_put2: FULLNAME=", pw->pw_gecos, ": ");
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1) {
		strerr_warn3("proxylogin: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
		return (1);
	}
	strnum1[i = fmt_ulong(strnum1, size_limit)] = 0;
	strnum2[j = fmt_ulong(strnum1, count_limit)] = 0;
	if (!stralloc_copyb(&tmpbuf, strnum1, i) ||
			!stralloc_catb(&tmpbuf, "S,", 2) ||
			!stralloc_catb(&tmpbuf, strnum2, j) ||
			!stralloc_append(&tmpbuf, "C") ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#else
	if (!stralloc_copys(&tmpbuf, str_diffn(pw->pw_shell, "NOQUOTA", 8) ? pw->pw_shell : "0") ||
			!stralloc_append(&tmpbuf, "S")
			!stralloc_0(&tmpbuf))
		die_nomem();
#endif
	if (!env_put2("MAILDIRQUOTA", tmpbuf.s))
		strerr_die3sys(111, "proxylogin: env_put2: MAILDIRQUOTA=", tmpbuf.s, ": ");
	if (!env_put2("HOME", pw->pw_dir))
		strerr_die3sys(111, "proxylogin: env_put2: HOME=", pw->pw_dir, ": ");
	if (!env_put2("AUTHSERVICE", service))
		strerr_die3sys(111, "proxylogin: env_put2: AUTHSERVICE=", service, ": ");
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
						strerr_warn3("proxylogin: open: ", ptr, ": ", &strerr_sys);
					else {
						substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
						for (;;) {
							if (getln(&ssin, &line, &match, '\n') == -1) {
								strerr_warn3("proxylogin: read: ", ptr, ": ", &strerr_sys);
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
				return (1);
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
	if (chdir(Maildir.s)) {
		strerr_warn3("proxylogin: chdir: ", Maildir.s, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copy(&tmpbuf, &Maildir) ||
			!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!env_put2("MAILDIR", tmpbuf.s))
		strerr_die3sys(111, "proxylogin: env_put2: AUTHSERVICE=", service, ": ");
	execv(argv[1], argv + 1);
	strerr_warn3("proxylogin: exec: ", argv[1], ": ", &strerr_sys);
	return (1);
}

int
AuthModuser(int argc, char **argv, unsigned timeout, unsigned errsleep)
{
	int             rc, n;
	time_t          t, expinterval;
	char           *p;

	p = env_get("AUTHUSER");
	if (!p && argc && argv[0][0] == '/') {
		/*- Set AUTHUSER from argv[0] */
		if (!env_put2("AUTHUSER", argv[0]))
			strerr_die3sys(111, "proxylogin: env_put2: AUTHUSER=", argv[0], ": ");
	} else
	if (!p || *p != '/') {
		strerr_warn2(argv[0], ": AUTHUSER is not initialized to a complete path", 0);
		_exit(1);
	}
	if (!env_put("AUTHENTICATED="))
		die_nomem();
	if (argc < 2) {
		if (write(2, "AUTHFAILURE\n", 12) == -1)
			;
		_exit(1);
	}
	p = env_get("AUTHARGC");
	rc = p && *p && *p != '0' ? 0 : 1;
	strnum[fmt_uint(strnum, (unsigned int) argc)] = 0;
	if (!env_put2("AUTHARGC", strnum))
		strerr_die3sys(111, "proxylogin: env_put2: AUTHARGC=", strnum, ": ");
	for (n = 0; n < argc; n++) {
		p = strnum;
		p += fmt_strn(p, "AUTHARGV", 8);
		p += fmt_uint(p, (unsigned int) n);
		*p++ = 0;
		if (!env_put2(p, argv[n]))
			strerr_die4sys(111, "proxylogin: env_put2: ", p, argv[n], ": ");
	}
	if (rc == 0 && errsleep)
		sleep(errsleep);
	time(&t);
	p = env_get("AUTHEXPIRE");
	if (p && isdigit((int) (unsigned char) *p)) {
		expinterval = 0;
		do
			expinterval = expinterval * 10 + (*p++ - '0');
		while (isdigit((int) (unsigned char) *p));
	} else {
		expinterval = t + timeout;
		strnum[fmt_ulong(strnum, (unsigned long) (t + timeout))] = 0;
		if (!env_put2("AUTHEXPIRE", strnum))
			strerr_die3sys(111, "proxylogin: env_put2: AUTHEXPIRE=", strnum, ": ");
	}
	if (timeout) {
		if (expinterval <= t)
			_exit(1);
		alarm(expinterval - t);
	}
	return (rc);
}
#endif

static int
have_imap_starttls()
{
	const char     *p;

	if ((p = env_get("IMAP_STARTTLS")) == 0)
		return (0);
	if (*p != 'y' && *p != 'Y')
		return (0);
	p = env_get("COURIERTLS");
	if (!p || !*p)
		return (0);
	if (access(p, X_OK))
		return (0);
	return (1);
}


static int
tlsrequired()
{
	char           *p = env_get("IMAP_TLS_REQUIRED");
	int             t;

	if (p) {
		scan_int(p, &t);
		if (t)
			return (1);
	}
	return (0);
}

void
imapd_capability()
{
	char           *p;
	int             t;

	out("proxylogin", "* CAPABILITY ");
	if ((p = env_get("IMAP_TLS")))
		scan_int(p, &t);
	if (p && t && (p = env_get("IMAP_CAPABILITY_TLS")) && *p)
		out("proxylogin", p);
	else
	if ((p = env_get("IMAP_CAPABILITY")) != 0 && *p)
		out("proxylogin", p);
	else
		out("proxylogin", "IMAP4rev1");

	if (have_imap_starttls())
	{
		out("proxylogin", " STARTTLS");
		if (tlsrequired())
			out("proxylogin", " LOGINDISABLED");
	}
	out("proxylogin", "\r\n");
	flush("proxylogin");
}

static int
have_pop3_starttls()
{
	char           *p;

	if (!(p = env_get("POP3_STARTTLS")))
		return (0);
	if (*p != 'y' && *p != 'Y')
		return (0);
	p = env_get("COURIERTLS");
	if (!p || !*p)
		return (0);
	if (access(p, X_OK))
		return (0);
	return (1);
}

void pop3d_capability()
{
	char           *p;
	int             t;

	out("proxylogin", "+OK Here's what I can do:\r\n");
	if ((p=env_get("POP3_TLS")))
		scan_int(p, &t);

	if (p && t && (p=env_get("POP3AUTH_TLS")) != 0 && *p)
		;
	else
		p=env_get("POP3AUTH");
	if (p && *p) {
		out("proxylogin", "SASL ");
		out("proxylogin", p);
		out("proxylogin", "\r\n");
	}
	if (have_pop3_starttls())
		out("proxylogin", "STLS\r\n");
	out("proxylogin", "TOP\r\nUSER\r\nLOGIN-DELAY 10\r\nPIPELINING\r\nUIDL\r\n.\r\n");
	flush("proxylogin");
}

/*
 * $Log: proxylogin.c,v $
 * Revision 1.13  2025-06-07 18:17:31+05:30  Cprogrammer
 * fixed gcc14 warning
 *
 * Revision 1.12  2023-07-16 13:59:07+05:30  Cprogrammer
 * check mkpasswd for error
 *
 * Revision 1.11  2023-07-15 12:52:17+05:30  Cprogrammer
 * authenticate using CRAM when password field starts with {CRAM}
 *
 * Revision 1.10  2023-01-03 21:48:21+05:30  Cprogrammer
 * added crlfile argument for auth_admin()
 *
 * Revision 1.9  2022-12-25 12:13:29+05:30  Cprogrammer
 * authenticate using SCRAM salted password
 *
 * Revision 1.8  2022-08-05 21:12:37+05:30  Cprogrammer
 * added encrypt_flag argument to autoAddUser()
 *
 * Revision 1.7  2021-07-22 15:17:27+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.6  2021-03-04 12:45:18+05:30  Cprogrammer
 * added option to specify CAFILE and match host with common name
 *
 * Revision 1.5  2020-10-01 18:28:17+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.4  2020-04-01 18:57:32+05:30  Cprogrammer
 * added encrypt flag to mkpasswd()
 *
 * Revision 1.3  2019-06-07 16:02:39+05:30  mbhangui
 * replaced getenv() with env_get()
 *
 * Revision 1.2  2019-04-22 23:14:42+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-18 15:48:27+05:30  Cprogrammer
 * Initial revision
 *
 */
