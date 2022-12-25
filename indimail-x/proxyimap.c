/*
 * $Log: proxyimap.c,v $
 * Revision 1.3  2022-12-25 20:32:53+05:30  Cprogrammer
 * allow any TLS/SSL helper program other than sslerator, couriertls
 *
 * Revision 1.2  2019-04-22 23:14:34+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 08:39:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: proxyimap.c,v 1.3 2022-12-25 20:32:53+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <strmsg.h>
#include <env.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <subfd.h>
#include <getln.h>
#include <substdio.h>
#endif
#include "AuthModuser.h"
#include "proxylogin.h"

static void     bye();
static char     strnum[FMT_ULONG + 11];
static stralloc tmpbuf = {0};

static void
die_nomem()
{
	strerr_warn1("proxyimap: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	static stralloc username = {0}, password = {0}, imaptag = {0}, dummy2 = {0}, line = {0};
	int             c, ret, match;
	char           *ptr, *cptr, *remoteip, *local_port, *destport;
	char           *tag = env_get("IMAPLOGINTAG");
	char           *binqqargs[7];

	if (argc != 3) {
		strerr_warn1("usage: proxyimap imapd Maildir", 0);
		return(1);
	}
	remoteip = local_port = destport = (char *) 0;
	if (!(remoteip = env_get("TCPREMOTEIP"))) {
		strerr_warn1("ERR: TCPREMOTEIP not set", 0);
		_exit(1);
	} else
	if (!(local_port = env_get("TCPLOCALPORT"))) {
		strerr_warn1("ERR: TCPLOCALPORT not set", 0);
		_exit(1);
	} else
	if (!(destport = env_get("DESTPORT"))) {
		strerr_warn1("ERR: DESTPORT not set", 0);
		_exit(1);
	} 
	signal(SIGALRM, bye);
	if (AuthModuser(argc, argv, 60, 5)) {
		strmsg_out1("* OK IMAP4rev1 Server Ready.\r\n");
		strerr_warn3("INFO: Connection, remoteip=[", remoteip, "]", 0);
		if (!env_put2("BADLOGINS", "0"))
			die_nomem();
	} else {
		if (!(ptr = (char *) env_get("BADLOGINS"))) {
			c = 0;
			if (!env_put2("BADLOGINS", "1"))
				die_nomem();
		} else {
			scan_uint(ptr, (unsigned int *) &c);
			strnum[fmt_uint(strnum, (unsigned int) (c + 1))] = 0;
			if (!env_put2("BADLOGINS", strnum))
				die_nomem();
		}
		if (c > 3)
			_exit(1);
		strmsg_out2(tag ? tag : "", tag ? " NO Login failed.\r\n" : "NO Login failed.\r\n");
		strerr_warn5("ERR: LOGIN FAILED, remoteip=[", remoteip, "] badlogin=[", strnum, "]", 0);
	}
	alarm(0);
	for (;;) {
		alarm(60);
		if (getln(subfdinsmall, &line, &match, '\n') == -1) {
			strerr_warn1("proxyimap: read-stdin: ", &strerr_sys);
			return (-1);
		}
		if (!match || !line.len)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		alarm(0);
		ret = 0;
		/*- imaptag */
		for (ptr = line.s; *ptr && isspace(*ptr); ptr++);
		for (c = 0, cptr = ptr; *ptr && !isspace(*ptr); c++, ptr++);
		if (c) {
			if (!stralloc_copyb(&imaptag, cptr, c) ||
					!stralloc_0(&imaptag))
				die_nomem();
			imaptag.len--;
			ret++;
		}

		/*- dummy2 */
		for (; *ptr && isspace(*ptr); ptr++);
		for (c = 0, cptr = ptr; *ptr && !isspace(*ptr); c++, ptr++);
		if (c) {
			if (!stralloc_copyb(&dummy2, cptr, c) ||
					!stralloc_0(&dummy2))
				die_nomem();
			dummy2.len--;
			ret++;
		}

		/*- username */
		for (; *ptr && isspace(*ptr); ptr++);
		for (c = 0, cptr = ptr; *ptr && !isspace(*ptr); c++, ptr++);
		if (c) {
			if (!stralloc_copyb(&username, cptr, c) ||
					!stralloc_0(&username))
				die_nomem();
			username.len--;
			ret++;
		}

		/*- password */
		for (; *ptr && isspace(*ptr); ptr++);
		for (c = 0, cptr = ptr; *ptr && !isspace(*ptr); c++, ptr++);
		if (c) {
			if (!stralloc_copyb(&password, cptr, c) ||
					!stralloc_0(&password))
				die_nomem();
			password.len--;
			ret++;
		}

		if (ret >= 2) {
			for (ptr = dummy2.s;*ptr;ptr++) {
				if (islower((int) *ptr))
					*ptr = toupper((int) *ptr);
			}
			if (!str_diff(dummy2.s, "LOGIN")) /*- proxylogin should normally never return */
				return(proxylogin(argv, destport, username.s, password.s, remoteip, imaptag.s, 1));
			else
			if (!str_diff(dummy2.s, "NOOP")) {
				strmsg_out2(imaptag.s, " OK NOOP completed\r\n");
				continue;
			} else
			if (!str_diff(dummy2.s, "STARTTLS")) {
				char           *p;

				if (!env_put2("AUTHARGC", "0"))
					die_nomem();
				for (c = 0; c < argc; c++) {
					p = strnum;
					p += fmt_strn(p, "AUTHARGV", 8);
					p += fmt_uint(p, (unsigned int) c);
					*p++ = 0;
					if (!env_unset(strnum))
						die_nomem();
				}
				if (!env_unset("BADLOGINS") || !env_unset("IMAP_STARTTLS"))
					die_nomem();
				alarm(0);
				if (!(ptr = env_get("COURIERTLS")))
					ptr = PREFIX"/bin/sslerator";
				c = str_rchr(ptr, '/');
				p = ptr[c] ? ptr + c + 1 : ptr;
				if (str_diff(p, "couriertls")) {
					binqqargs[0] = ptr;
					binqqargs[1] = argv[0];
					binqqargs[2] = argv[1]; 
					binqqargs[3] = argv[2];
					binqqargs[4] = 0;
					if (!stralloc_copy(&tmpbuf, &imaptag) ||
							!stralloc_catb(&tmpbuf, " Begin SSL/TLS negotiation now.\r\n", 33) ||
							!stralloc_0(&tmpbuf))
						die_nomem();
					if (!env_put2("BANNER", tmpbuf.s))
						die_nomem();
				} else {
					if (!env_unset("COURIERTLS"))
						die_nomem();
					binqqargs[0] = ptr;
					binqqargs[1] = "-remotefd=0";
					binqqargs[2] = "-server";
					binqqargs[3] = argv[0];
					binqqargs[4] = argv[1]; 
					binqqargs[5] = argv[2];
					binqqargs[6] = 0;
					strmsg_out2(imaptag.s, " Begin SSL/TLS negotiation now.\r\n");
				}
				execv(*binqqargs, binqqargs);
				strerr_warn3("proxyimap: execv: ", *binqqargs, ": ", &strerr_sys);
				continue;
			} else
			if (!str_diff(dummy2.s, "CAPABILITY")) {
				imapd_capability();
				strmsg_out2(imaptag.s, " OK CAPABILITY completed\r\n");
				continue;
			} else
			if (!str_diff(dummy2.s, "LOGOUT")) {
				strmsg_out1("* BYE IMAP4rev1 server shutting down\r\n");
				strmsg_out1(" OK LOGOUT completed\r\n");
				break;
			}
		}
		strmsg_out1("* NO Error in IMAP command received by server.\n");
	}
	return(0);
}

static void bye()
{
	strerr_warn1("ERR: TIMEOUT", 0);
	_exit(0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return(0);
}
#endif
