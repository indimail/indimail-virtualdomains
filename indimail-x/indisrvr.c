/*
 * $Log: indisrvr.c,v $
 * Revision 1.12  2023-01-03 21:35:28+05:30  Cprogrammer
 * replaced tls code with TLS library from libqmail
 * added -T option to specify connection timeout
 * added -r option to specify crl file
 * added -d option to specify certificate dir
 *
 * Revision 1.11  2022-12-18 19:24:01+05:30  Cprogrammer
 * log additional wait status
 *
 * Revision 1.10  2022-10-20 11:57:41+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.9  2022-05-10 20:00:52+05:30  Cprogrammer
 * use headers from include path
 *
 * Revision 1.8  2021-06-11 17:03:47+05:30  Cprogrammer
 * replaced MakeArgs() with makeargs() from libqmail()
 *
 * Revision 1.7  2021-03-09 19:58:25+05:30  Cprogrammer
 * use functions from tls.c
 *
 * Revision 1.6  2021-03-09 15:33:58+05:30  Cprogrammer
 * renamed SSL_CIPHER to TLS_CIPHER_LIST
 *
 * Revision 1.5  2020-10-01 18:23:48+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.4  2020-04-01 18:55:43+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-06-07 16:00:18+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:11:33+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-18 08:23:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: indisrvr.c,v 1.12 2023-01-03 21:35:28+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <tls.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <env.h>
#include <strerr.h>
#include <error.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#include <pw_comp.h>
#include <tcpbind.h>
#include <makeargs.h>
#endif
#include "variables.h"
#include "checkPerm.h"
#include "mgmtpassfuncs.h"
#include "filewrt.h"

#define MAXBUF 4096

/*
 * adminclient Protocol
 *
 * s: "Login: "
 * c: "userid\n"
 * s: "Password: "
 * c: "password\n"
 * s: "OK\n"
 * c: "index command arg1 arg2 ...\n"
 * s: <output of above command if any>
 * c: "\n"
 * s: "RETURNSTATUS[return value of command]\n"
 *
 * e.g.
 *
 * Login: admin<lf>
 * Password: xxxxxxxx<lf>
 * OK
 * 7 vuserinfo -n manvendra@indimail.org<lf>
 * name          : manvendra@indimail.org
 * <lf>
 * RETURNSTATUS0
 *
 */
int             Login_User(stralloc *, stralloc *);
int             call_prg();
static void     SigChild(void);
static void     SigTerm();
static void     SigUsr();
static int      get_options(int argc, char **argv, char **, char **, int *);
#ifdef HAVE_SSL
static void     SigHup();
#endif

#ifdef HAVE_SSL
SSL_CTX        *ctx;
SSL            *ssl;
static int      usessl = 0;
unsigned long   dtimeout = 300;
unsigned long   ctimeout = 60;
static char    *certfile, *cafile, *crlfile, *certdir;
#endif

char            tbuf[2048];

static void
die_nomem()
{
	strerr_warn1("indisrvr: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	int             n, socket_desc, pid, backlog;
	char           *port, *ipaddr;
	struct sockaddr_in cliaddress;
	int             addrlen, len, new;
	struct linger   linger;
#ifdef ENABLE_IPV6
	char            hostname[256], servicename[100];
#endif
#ifdef HAVE_SSL
	char           *ciphers = NULL;
	int             r, status, retval, pi1[2], pi2[2];
#endif

	if (get_options(argc, argv, &ipaddr, &port, &backlog))
		return (1);
	/*
	 * dup fd 2 to 3 use 3 in child to print
	 * erors on parents original error stream,
	 * allowing child's errors to be logged to
	 * parent's fd 2
	 */
	dup2(2, 3);
	(void) signal(SIGTERM, SigTerm);
	(void) signal(SIGUSR2, SigUsr);
#ifdef HAVE_SSL
	if (usessl == 1) {
		set_certdir(certdir);
		if (!certfile)
			strerr_die1x(1, "indisrvr: certificate must be provided");
		else
		if (access(certfile, R_OK))
			strerr_die3sys(1, "indisrvr: ", certfile, ": ");
    	/* setup SSL context (load key and cert into ctx) */
		if (!(ciphers = env_get("TLS_CIPHER_LIST")))
			ciphers = "PROFILE=SYSTEM";
		if (!(ctx = tls_init(0, certfile, cafile, crlfile, ciphers, server)))
			return (1);
		(void) signal(SIGHUP, SigHup);
	}
#endif
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if ((socket_desc = tcpbind(ipaddr, port, backlog)) == -1) {
		strerr_warn1("indisrvr: tcpbind: ", &strerr_sys);
		return (1);
	}
	len = MAXBUF; /*- for setsockopt */
	addrlen = sizeof(cliaddress);
	(void) signal(SIGCHLD, (void (*)()) SigChild);
#ifdef HAVE_SSL
	filewrt(3, "%d: IndiServer Ready with Address %s:%s backlog %d cert=%s\n", getpid(), ipaddr, port, backlog, certfile);
#else
	filewrt(3, "%d: IndiServer Ready with Address %s:%s backlog %d SSL=%d\n", getpid(), ipaddr, port, backlog, 0);
#endif
	close(0);
	close(1);
	for (;;) {
		if ((new = accept(socket_desc, (struct sockaddr *) &cliaddress, (socklen_t *) &addrlen)) == -1) {
			switch (errno)
			{
			case EINTR:
#ifdef ERESTART
			case ERESTART:
#endif
				continue;
			default:
				strerr_warn1("indisrver: accept: ", &strerr_sys);
				close(socket_desc);
				_exit(1);
			}
			break;
		}
#ifdef ENABLE_IPV6
		*hostname = 0;
		if (!(n = getnameinfo((struct sockaddr *) &cliaddress, addrlen,
			hostname, sizeof(hostname), servicename, sizeof(servicename), NI_NUMERICHOST|NI_NUMERICSERV)))
			filewrt(3, "%d: Connection from %s:%s\n", getpid(), hostname, servicename);
		else
			strerr_warn2("indisrver: getnameinfo: ", (char *) gai_strerror(n), 0);
#else
		filewrt(3, "%d: Connection from %s:%d\n", getpid(), inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
#endif
		switch (pid = fork())
		{
		case -1:
			close(new);
			continue;
		case 0: /*- socket handling child */
			(void) signal(SIGTERM, SIG_DFL);
			(void) signal(SIGCHLD, SIG_IGN);
			(void) signal(SIGHUP, SIG_IGN);
			close(socket_desc);
			if (setsockopt(new, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)) == -1) {
				close(new);
				_exit(1);
			}
			for (;;) {
				if (setsockopt(new, SOL_SOCKET, SO_SNDBUF, (void *) &len, sizeof(int)) == -1) {
					if (errno == ENOBUFS) {
						usleep(1000);
						continue;
					}
					close(new);
					_exit(1);
				}
				break;
			}
			for (;;) {
				if (setsockopt(new, SOL_SOCKET, SO_RCVBUF, (void *) &len, sizeof(int)) == -1) {
					if (errno == ENOBUFS) {
						usleep(1000);
						continue;
					}
					close(new);
					_exit(1);
				}
				break;
			}
			if (dup2(new, 0) == -1 || dup2(new, 1) == -1 || dup2(new, 2) == -1)
				strerr_die1sys(1, "indisrver: dup2 (0, 1, 2): ");
			if (new != 0 && new != 1 && new != 2)
				close(new);
#ifdef HAVE_SSL
			if (usessl == 1) {
				if (pipe(pi1) != 0 || pipe(pi2) != 0)
					strerr_die1sys(1, "unable to create pipe: ");
				switch (fork())
				{
				case 0: /* command handlng child */
					SSL_CTX_free(ctx);
					close(pi1[1]);
					close(pi2[0]);
					if (dup2(pi1[0], 0) == -1 || dup2(pi2[1], 1) == -1 || dup2(pi2[1], 2) == -1)
						strerr_die1sys(1, "unable to setup descriptors: ");
					if (pi1[0] != 0)
						close(pi1[0]);
					if (pi2[1] != 1 && pi2[1] != 2)
						close(pi2[1]);
					/*- signals are allready set in the parent */
					n = call_prg();
					close(0);
					close(1);
					close(2);
					close(3);
					_exit(n);
				case -1:
					strerr_die1sys(1, "fork: ");
				default:
					break;
				} /*- switch (fork()) */
				close(pi1[0]);
				close(pi2[1]);
				if (!(ssl = tls_session(ctx, 1))) {
					filewrt(3, "%d: unable to setup SSL session: %s\n", getpid(), myssl_error_str());
					_exit(1);
				}
				SSL_CTX_free(ctx);
				if (tls_accept(ctimeout, 0, 1, ssl)) {
					ssl = 0;
					filewrt(3, "%d: unable to accept SSL connection: %s\n", getpid(), myssl_error_str());
					_exit(1);
				}
				n = translate(0, 1, pi1[1], pi2[0], dtimeout);
				ssl_free();
				ssl = 0;
				for (retval = -1;(r = waitpid(pid, &status, WNOHANG | WUNTRACED));) {
#ifdef ERESTART
					if (r == -1 && (errno == EINTR || errno == ERESTART))
#else
					if (r == -1 && errno == EINTR)
#endif
						continue;
					if (WIFSTOPPED(status) || WIFCONTINUED(status)) {
						if (verbose)
							filewrt(3, "%d: %s by signal %d\n", pid, WIFSTOPPED(status) ? "stopped" : "started",
									WIFSTOPPED(status) ? WSTOPSIG(status) : SIGCONT);
						continue;
					} else
					if (WIFSIGNALED(status)) {
						if (verbose)
							filewrt(3, "%d: killed by signal %d\n", pid, WTERMSIG(status));
						retval = -1;
					} else
					if (WIFEXITED(status)) {
						retval = WEXITSTATUS(status);
						if (verbose)
							filewrt(3, "%d: normal exit return status %d\n", pid, retval);
					}
					break;
				} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
				close(0);
				close(1);
				close(2);
				close(3);
				if (n)
					_exit(n);
				if (retval)
					_exit(retval);
				_exit (0);
			} else { /*- not ssl */
				n = call_prg();
				close(0);
				close(1);
				close(2);
				close(3);
				_exit(n);
			}
#else
			n = call_prg();
			close(0);
			close(1);
			close(2);
			close(3);
			_exit(n);
#endif
		default:
			close(new);
			break;
		} /*- switch (pid = fork()) */
	} /*- for (;;) */
	_exit(1);
}

int
call_prg()
{
	char           *ptr;
	char          **Argv;
	int             i, status, cmdcount, retval, match;
	static stralloc username = {0}, pass = {0}, line = {0};
	pid_t           pid;
	char            strnum[FMT_ULONG];

	if ((i = Login_User(&username, &pass)) == -1) {
		strerr_warn1("temporary authentication error", 0);
		return (-1);
	} else
	if (i)
		return (1);
	if (getln(subfdinsmall, &line, &match, '\n') == -1) {
		strerr_warn1("indisrvr: read stdin: ", &strerr_sys);
		return (-1);
	}
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	}
	scan_int(line.s, &i);
	for (cmdcount = 0; adminCommands[cmdcount].name; cmdcount++);
	match = str_chr(line.s, ' ');
	if (i > cmdcount || !line.s[match]) {
		filewrt(2, "indisrvr: incorrect syntax %d[%s]\n", i, line.s);
		filewrt(3, "indisrvr: %d: incorrect syntax %d[%s]\n", getpid(), i, line.s);
		return (1);
	}
	ptr = line.s + match + 1; /*- command */
	(void) signal(SIGCHLD, SIG_DFL);
	switch (pid = fork())
	{
	case -1:
		strerr_warn1("indisrvr: fork: ", &strerr_sys);
		filewrt(3, "%d: fork: %s\n", getpid(), error_str(errno));
		return (-1);
	case 0:
		(void) signal(SIGCHLD, SIG_DFL);
		if (!(Argv = makeargs(ptr))) {
			strerr_warn1("makeargs failed: ", &strerr_sys);
			filewrt(3, "%d: makeargs failed: %s\n", getpid(), error_str(errno));
			return (-1);
		}
		if (checkPerm(username.s, adminCommands[i].name, Argv)) {
			strerr_warn6(username.s, ": ", adminCommands[i].name, " args [", ptr, "]: permission denied", 0);
			filewrt(3, "%s: %s args [%s]: permission denied\n", username.s, adminCommands[i].name, ptr);
			_exit(1);
		}
		if (verbose)
			filewrt(3, "%d: command %s args %s\n", getpid(), adminCommands[i].name, ptr);
		execv(adminCommands[i].name, Argv);
		filewrt(3, "%d: %s args [%s]: %s\n", getpid(), adminCommands[i].name, ptr, error_str(errno));
		_exit(1);
	default:
		break;
	}
	for (retval = -1;;) {
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (WIFSTOPPED(status) || WIFCONTINUED(status)) {
			if (verbose)
				filewrt(3, "%d: %s by signal %d\n", pid, WIFSTOPPED(status) ? "stopped" : "started",
					WIFSTOPPED(status) ? WSTOPSIG(status) : SIGCONT);
			continue;
		} else
		if (WIFSIGNALED(status)) {
			if (verbose)
				filewrt(3, "%d: killed by signal %d\n", pid, WTERMSIG(status));
			retval = -1;
		} else
		if (WIFEXITED(status)) {
			retval = WEXITSTATUS(status);
			if (verbose)
				filewrt(3, "%d: normal exit return status %d\n", pid, retval);
		}
		break;
	}
	if (getln(subfdinsmall, &line, &match, '\n') == -1) {
		i = errno;
		strerr_warn1("indisrvr: read stdin: ", &strerr_sys);
		filewrt(3, "imdisrvr: read failed: %s\n", error_str(i));
		return (-1);
	}
	if (verbose)
		filewrt(3, "%d: return status %d\n", pid, retval);
	strnum[i = fmt_int(strnum, retval)] = 0;
	if (substdio_put(subfdoutsmall, "RETURNSTATUS", 12) ||
			substdio_put(subfdoutsmall, strnum, i) ||
			substdio_flush(subfdoutsmall)) {
		strerr_warn1("indisrvr: write stdout: ", &strerr_sys);
		return (-1);
	}
	return (0);
}

/*
 * return to parent (call_prg) for:
 *  0 - for success
 *  & exit for the foll.
 *  1.) Mysql Prb , 2.) Password incorrect & 3.) If the user is not present
 */
int
Login_User(stralloc *username, stralloc *password)
{
	char           *admin_pass;
	int             match;

	if (substdio_put(subfdoutsmall, "Login: ", 7) ||
			substdio_flush(subfdoutsmall)) {
		strerr_warn1("indisrvr: write stdout: ", &strerr_sys);
		return (1);
	}
	if (getln(subfdinsmall, username, &match, '\n') == -1) {
		strerr_warn1("indisrvr: read stdin: ", &strerr_sys);
		return (-1);
	}
	if (match) {
		username->len--;
		username->s[username->len] = 0;
	} else {
		if (!stralloc_0(username))
			die_nomem();
		username->len--;
	}
	if (substdio_put(subfdoutsmall, "Password: ", 10) ||
			substdio_flush(subfdoutsmall)) {
		strerr_warn1("indisrvr: write stdout: ", &strerr_sys);
		return (1);
	}
	if (getln(subfdinsmall, password, &match, '\n') == -1) {
		strerr_warn1("indisrvr: read stdin: ", &strerr_sys);
		return (-1);
	}
	if (match) {
		password->len--;
		password->s[password->len] = 0;
	} else {
		if (!stralloc_0(password))
			die_nomem();
		password->len--;
	}
	if (isDisabled(username->s))
	{
		filewrt(3, "%d: user %s disabled\n", getpid(), username->s);
		strerr_warn1("You are disabled", 0);
		return (1);
	}
	if (!(admin_pass = mgmtgetpass(username->s, 0)))
		return (1);
	if (*admin_pass && !pw_comp(0, (unsigned char *) admin_pass, 0,
		(unsigned char *) password->s, 0)) {
		if (substdio_put(subfdoutsmall, "OK\n", 3) ||
				substdio_flush(subfdoutsmall)) {
			strerr_warn1("indisrvr: write stdout: ", &strerr_sys);
			return (1);
		}
		filewrt(3, "%d: user %s logged in\n", getpid(), username->s);
		return (0);
	}
	filewrt(3, "%d: user %s password incorrect\n", getpid(), username->s);
	strerr_warn1("indisrver: Password incorrect", 0);
	if (updateLoginFailed(username->s)) {
		strerr_warn1("failed to update login status", 0);
	}
	return (1);
}

static int
get_options(int argc, char **argv, char **ipaddr, char **port, int *backlog)
{
	int             c;

#ifdef HAVE_SSL
	certfile = 0;
	certdir = SYSCONFDIR"/certs";
#endif
	*ipaddr = *port = 0;
	*backlog = -1;
#ifdef HAVE_SSL
	while ((c = getopt(argc, argv, "vt:T:i:p:b:n:c:r:d:")) != opteof)
#else
	while ((c = getopt(argc, argv, "vt:T:i:p:b:")) != opteof)
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'i':
			*ipaddr = optarg;
			break;
		case 'p':
			*port = optarg;
			break;
		case 't':
			scan_ulong(optarg, &dtimeout);
			break;
		case 'T':
			scan_ulong(optarg, &ctimeout);
			break;
		case 'b':
			scan_int(optarg, backlog);
			break;
#ifdef HAVE_SSL
		case 'd':
			certdir = optarg;
			break;
		case 'n':
			usessl = 1;
			certfile = optarg;
			break;
		case 'c':
			cafile = optarg;
			break;
		case 'r':
			crlfile = optarg;
			break;
#endif
		default:
#ifdef HAVE_SSL
			strerr_warn1("usage: indisrvr -i ipaddr -p port [-d certdir] -n certfile [-c cafile -r crlfile] -t timeoutdata -T timeoutconn -b backlog", 0);
#else
			strerr_warn1("usage: indisrvr -i ipaddr -p port -t timeoutdata -T timeoutconn -b backlog", 0);
#endif
			break;
		}
	}
	if (!*ipaddr || !*port || *backlog == -1)
	{
#ifdef HAVE_SSL
		strerr_warn1("usage: indisrvr -i ipaddr -p port [-d certdir] -n certfile [-c cafile -r crlfile] -t timeoutdata -T timeoutconn -b backlog", 0);
#else
		strerr_warn1("usage: indisrvr -i ipaddr -p port -t timeoutdata -T timeoutconn -b backlog", 0);
#endif
		return (1);
	}
	return (0);
}

static void
SigTerm()
{
	filewrt(3, "%d: indisrvr going down on SIGTERM\n", getpid());
	_exit(0);
}

static void
SigChild(void)
{
	int             status;
	pid_t           pid;

	for (;(pid = waitpid(-1, &status, WNOHANG | WUNTRACED));)
	{
#ifdef ERESTART
		if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (pid == -1 && errno == EINTR)
#endif
			continue;
		break;
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	return;
}

static void
SigUsr(void)
{
	filewrt(3, "%d Resetting Verbose flag to %d\n", (int) getpid(), verbose ? 0 : 1);
	verbose = (verbose ? 0 : 1);
	(void) signal(SIGUSR2, (void(*)()) SigUsr);
	errno = EINTR;
	return;
}

#ifdef HAVE_SSL
static void
SigHup(void)
{
	filewrt(3, "%d: IndiServer received SIGHUP\n", getpid());
	if (ctx)
		SSL_CTX_free(ctx);
	if (ssl)
		SSL_free(ssl);
	ctx = NULL;
	ssl = NULL;
	(void) signal(SIGHUP, (void(*)()) SigHup);
	errno = EINTR;
	return;
}
#endif
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (1);
}
#endif
