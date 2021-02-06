/*
 * $Log: proxypop3.c,v $
 * Revision 1.3  2019-04-22 23:14:49+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.2  2019-04-18 15:41:42+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: proxypop3.c,v 1.3 2019-04-22 23:14:49+05:30 Cprogrammer Exp mbhangui $";
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

static void
die_nomem()
{
	strerr_warn1("proxypop3: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *p, *q, *r, *remoteip, *local_port, *destport = 0;
	static stralloc user = {0}, pass = {0}, line = {0};
	int             c, match;
	char           *binqqargs[7];

	if (argc != 3) {
		strerr_warn1("usage: proxypop3 pop3d Maildir", 0);
		return (1);
	}
	if (!(remoteip = env_get("TCPREMOTEIP"))) {
		strerr_warn1("ERR: TCPREMOTEIP not set", 0);
		_exit(1);
	} else
	if (!(local_port = env_get("TCPLOCALPORT"))) {
		strerr_warn1("ERR: TCPLOCALPORT not set", 0);
		_exit(1);
	}
	if (!(destport = env_get("DESTPORT"))) {
		strerr_warn1("ERR: DESTPORT not set", 0);
		_exit(1);
	} 
	signal(SIGALRM, bye);
	if (AuthModuser(argc, argv, 60, 5)) {
		if (!env_get("SSLERATOR")) {
			strerr_warn3("INFO: Connection, remoteip=[", remoteip, "]", 0);
			strmsg_out1("+OK POP3 Server Ready.\r\n");
		}
		if (!env_put2("BADLOGINS", "0"))
			die_nomem();
	} else {
		if (!(p = (char *) env_get("BADLOGINS"))) {
			c = 0;
			if (!env_put2("BADLOGINS", "1"))
				die_nomem();
		} else {
			scan_uint(p, (unsigned int *) &c);
			strnum[fmt_uint(strnum, (unsigned int) (c + 1))] = 0;
			if (!env_put2("BADLOGINS", strnum))
				die_nomem();
		}
		if (c > 3)
			_exit(1);
		strerr_warn3("ERR: LOGIN FAILED, remoteip=[", remoteip, "]", 0);
		strmsg_out1("-ERR Login failed.\r\n");
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
			if (line.s[line.len - 1] == '\r')
				line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		alarm(0);
		for (p = line.s; *p && isspace(*p); p++);
		for (c = 0, q = p; *q && !isspace(*q); c++, q++) {
			if (islower((int) *q))
				*q = toupper((int) *q);
		}
		if (!*p) {
			strmsg_out1("-ERR Invalid command.\r\n");
			continue;
		}
		p[c] = 0;
		for (q += 1; *q && isspace(*q); q++);
		if (!str_diff(p, "QUIT")) {
			strmsg_out1("+OK Phir Khab Miloge.\r\n");
			break;
		} else
		if (!str_diff(p, "NOOP") || !str_diff(p, "RSET") || !str_diff(p, "STAT") || !str_diff(p, "LIST") ||
				!str_diff(p, "RETR") || !str_diff(p, "DELE") || !str_diff(p, "TOP") || !str_diff(p, "UIDL"))
		{
			strmsg_out1("-ERR command valid only in transaction state.\r\n");
			continue;
		} else
		if (!str_diff(p, "CAPA")) {
			pop3d_capability();
			continue;
		} else
		if (!str_diff(p, "STLS")) {
			char           *ptr;

			if (!env_put2("AUTHARGC", "0"))
				die_nomem();
			for (c = 0; c < argc; c++) {
				q = strnum;
				q += fmt_strn(q, "AUTHARGV", 8);
				q += fmt_uint(q, (unsigned int) c);
				*q++ = 0;
				if (!env_unset(strnum))
					die_nomem();
			}
			if (!env_unset("BADLOGINS") || !env_unset("POP3_STARTTLS"))
				die_nomem();
			alarm(0);
			r = ptr = env_get("COURIERTLS");
			if (ptr) {
				c = str_rchr(ptr, '/');
				if (ptr[c])
					r = ptr + c + 1;
			} else
				r = 0;

			if (!ptr || (r && !str_diff(r, "sslerator"))) {
				binqqargs[0] = PREFIX"/bin/sslerator";
				binqqargs[1] = argv[0];
				binqqargs[2] = argv[1]; 
				binqqargs[3] = argv[2];
				binqqargs[4] = 0;
				if (!env_put2("BANNER", "+OK Begin SSL/TLS negotiation now.\r\n"))
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
				if (!env_put2("SSLERATOR", "1"))
					die_nomem();
				strmsg_out1("+OK Begin SSL/TLS negotiation now.\r\n");
			}
			execv(*binqqargs, binqqargs);
			strerr_warn3("proxyimap: execv: ", *binqqargs, ": ", &strerr_sys);
			continue;
		} else
		if (!str_diff(p, "USER")) {
			c = line.len - (q - line.s);
			if (c && (!stralloc_copyb(&user, q, c) || !stralloc_0(&user)))
				die_nomem();
			user.len--;
			if (!c) {
				strmsg_out1("-ERR USER/PASS required.\r\n");
				continue;
			}
			strmsg_out1("+OK Password required.\r\n");
			continue;
		} else
		if (!str_diff(p, "PASS")) {
			c = line.len - (q - line.s);
			if (!stralloc_copyb(&pass, q, c) || !stralloc_0(&pass))
				die_nomem();
			pass.len--;
			if (!user.len || !c) {
				strmsg_out1("-ERR USER/PASS required.\r\n");
				continue;
			}
		} else {
			strmsg_out1("-ERR Invalid command.\r\n");
			continue;
		} /*- proxylogin should normally never return */
		return (proxylogin(argv, destport, user.s, pass.s, remoteip, 0, 2));
	} /* while (alarm(300), fgets(buf, sizeof(buf), stdin)) */
	return (0);
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
	return (0);
}
#endif
