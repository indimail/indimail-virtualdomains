#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_RPC_RPC_H
#include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif

/*- tirpc */
#ifdef HAVE_TIRPC
#ifndef HAVE_RPC_RPC_H
#ifdef HAVE_TIRPC_RPC_RPC_H
#include <tirpc/rpc/rpc.h>
#endif /*- #ifdef HAVE_TIRPC_RPC_RPC_H */
#endif /*- #ifdef HAVE_RPC_RPC_H */
#ifndef HAVE_RPC_TYPES_H
#ifdef HAVE_TIRPC_RPC_TYPES_H
#include <tirpc/rpc/types.h>
#endif /*- #ifdef HAVE_TIRPC_RPC_TYPES_H */
#endif /*- #ifndef HAVE_RPC_TYPES_H */
#endif /*- #ifdef HAVE_TIRPC */

#include <signal.h>
#include <errno.h>
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <tls.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <subfd.h>
#include <getEnvConfig.h>
#include <tcpbind.h>
#include <sgetopt.h>
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <getln.h>
#include <qprintf.h>
#include <timeoutwrite.h>
#include <scan.h>
#include <open.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: logsrv.c,v 1.22 2023-04-14 09:43:27+05:30 Cprogrammer Exp mbhangui $";
#endif

/*-
program RPCLOG
{
	version CLOGVERS
	{
		int	SEND_MESSAGE( string ) = 1;
	} = 1;
} = 0x20000001;
-*/


#define RPCLOG ((u_long)0x20000001)
#define CLOGVERS ((u_long)1)
#define SEND_MESSAGE ((u_long)1)

#define WARN      "logsrv: warn: "
#define FATAL     "logsrv: fatal: "

#if !defined(HAVE_TIRPC) && !defined(HAVE_RPC_RPC_H)
int
main(int argc, char **argv)
{
	strerr_die2x(111, FATAL, "no usage rpc/svc library found");
}
#else
char            strnum[FMT_ULONG];
char           *loghost;
CLIENT         *cl;
static char    *usage = "usage: logsrv [-v] [-c connect_timeout] [-d data_timeout] [-r host] dir";
static stralloc line = {0}, rhost = {0}, statusfn = {0}, tmp = {0};
unsigned long   ctimeout = 60, dtimeout = 300;

void
SigTerm()
{
	strerr_die2x(0, FATAL, "ARGH!! Committing suicide on SIGTERM");
}

void
write_bytes(int fd, size_t *bytes)
{
	if (lseek(fd, sizeof(pid_t), SEEK_SET) == -1)
		strerr_die2sys(111, FATAL, "unable to seek status file");
	if (write(fd, (char *) bytes, sizeof(size_t)) == -1)
		strerr_die2sys(111, FATAL, "unable to write to status file");
	return;
}

int            *
send_message_1(char **argp, CLIENT *clnt)
{
	static int      res;
	struct timeval  TIMEOUT = {25, 0};

	(void) memset((char *) &res, 0, sizeof(res));
	if (clnt_call(clnt, SEND_MESSAGE, (xdrproc_t) xdr_wrapstring, (char *) argp,
			(xdrproc_t) xdr_int, (char *) &res, TIMEOUT) != RPC_SUCCESS) {
		fprintf(stderr, "clnt_call: %s\n", clnt_sperror(clnt, loghost));
		return (NULL);
	}
	return (&res);
}

int
log_msg(char **str)
{
	int            *ret;
	static int      flag;

	if (!flag) {
		if (!(cl = clnt_create(loghost, RPCLOG, CLOGVERS, "tcp"))) {
			fprintf(stderr, "clnt_create: %s\n", clnt_spcreateerror(loghost));
			return (-1);
		}
		flag++;
	}
	if (!(ret = send_message_1(str, cl))) {
		clnt_perror(cl, "send_message_1");
		clnt_destroy(cl);
		flag = 0;
		return (-1);
	} else
		return (0);
}

void
do_server(int verbose, char *statusdir)
{
	int             match, fd, n;
	size_t          bytes;
	pid_t           pid;
	char           *(logline[1]), *ptr;
	char            inbuf[512];
	substdio        ssin;

	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
	if (getln(&ssin, &rhost, &match, '\0') == -1)
		strerr_die2sys(111, FATAL, "read: socket: ");
	if (!match)
		strerr_die2sys(111, FATAL, "read2: socket: ");
	if (!stralloc_0(&rhost))
		strerr_die2x(111, FATAL, "out of memory");
	rhost.len -= 2;
	if (timeoutwrite(dtimeout, 1, "1", 1) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	qsprintf(&statusfn, "%s/%s.status", statusdir, rhost.s);
	if (!access(statusfn.s, R_OK)) {
		if ((fd = open_readwrite(statusfn.s)) == -1)
			strerr_die4sys(111, FATAL, "unable to open for read+write: ", statusfn.s, ": ");
		if ((n = read(fd, (char *) &pid, sizeof(pid_t))) == -1)
			strerr_die4sys(111, FATAL, "unable to read pid from ", statusfn.s, ": ");
		if ((n = read(fd, (char *) &bytes, sizeof(bytes))) == -1)
			strerr_die4sys(111, FATAL, "unable to read offset from ", statusfn.s, ": ");
	} else {
		if ((fd = open(statusfn.s, O_CREAT|O_RDWR, 0644)) == -1)
			strerr_die4sys(111, FATAL, "unable to create: ", statusfn.s, ": ");
		bytes = 0;
	}
	if (lseek(fd, 0, SEEK_SET) == -1)
		strerr_die4sys(111, FATAL, "unable to seek ", statusfn.s, ": ");
	pid = getpid();
	if (write(fd, (char *) &pid, sizeof(pid_t)) != sizeof(pid_t))
		strerr_die4sys(111, FATAL, "unable to write pid to ", statusfn.s, ": ");
	subprintf(subfderr, "Connection request from %s\n", rhost.s);
	substdio_flush(subfderr);
	for (;;) {
		n = 0;
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "read3: socket: ");
		if (!line.len)
			break;
		if (!stralloc_0(&line))
			strerr_die2x(111, FATAL, "out of memory");
		line.len--;
		if (loghost) {
			qsprintf(&tmp, "%s %s", rhost.s, line.s);
			*logline = tmp.s;
			if (log_msg(logline) == -1) {
				shutdown(0, 0);
				return;
			}
		}

		if (verbose) {
			if (subprintf(subfderr, "%s %s", rhost.s, line.s) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			if (substdio_flush(subfderr) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		}
		if (timeoutwrite(dtimeout, 1, "1", 1) == -1)
			strerr_die2sys(111, FATAL, "write: ");
		for (n = 0,ptr = line.s; *ptr; ptr++) {
			if (*ptr == ' ')
				n++;
			if (n == 2)
				break;
		}
		if (*ptr) {
			bytes += (line.len - (ptr - line.s) - 1);
		} else {
			bytes += line.len;
			strerr_warn2(WARN, "error-%s", line.s);
		}
		write_bytes(fd, &bytes);
	} /* for (;;) */
	_exit(1);
}

int
main(int argc, char **argv)
{
	int             c, verbose = 0;

	while ((c = getopt(argc, argv, "vc:d:r:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'c':
			scan_ulong(optarg, &ctimeout);
			break;
		case 'd':
			scan_ulong(optarg, &dtimeout);
			break;
		case 'r':
			loghost = optarg;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (argc == optind)
		strerr_die1x(100, usage);
	signal(SIGTERM, SigTerm);
	do_server(verbose, argv[optind]);
}

#ifndef	lint
void
getversion_logsrv_c()
{
	printf("%s\n", sccsid);
}
#endif
#endif /*- #if !defined(HAVE_TIRPC) && !defined(HAVE_RPC_RPC_H) */

/*
 * $Log: logsrv.c,v $
 * Revision 1.22  2023-04-14 09:43:27+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.21  2022-12-06 12:24:59+05:30  Cprogrammer
 * removed filewrt to remove dependency on libindimail
 *
 * Revision 1.20  2022-05-21 11:11:36+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.19  2022-05-10 20:10:07+05:30  Cprogrammer
 * use tcpbind from libqmail
 *
 * Revision 1.18  2022-05-10 01:13:45+05:30  Cprogrammer
 * 'fixes for including rpc/tirpc headers'
 *
 * Revision 1.17  2021-04-05 21:58:03+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.16  2021-03-15 12:50:29+05:30  Cprogrammer
 * include filewrt.h
 *
 * Revision 1.15  2021-03-15 11:32:19+05:30  Cprogrammer
 * added sockread() function
 *
 * Revision 1.14  2020-06-21 12:49:10+05:30  Cprogrammer
 * quench rpmlint
 *
 * Revision 1.13  2018-08-21 19:38:58+05:30  Cprogrammer
 * fix for rpc.h on openSUSE tumbleweed
 *
 * Revision 1.12  2018-08-20 13:13:04+05:30  Cprogrammer
 * added tirpc/rpc inclusion
 *
 * Revision 1.11  2017-03-07 19:35:08+05:30  Cprogrammer
 * added CRYPTO_POLICY_NON_COMPLIANCE
 *
 * Revision 1.10  2016-06-21 13:30:57+05:30  Cprogrammer
 * use SSL_set_cipher_list as part of crypto-policy-compliance
 *
 * Revision 1.9  2014-04-17 11:27:43+05:30  Cprogrammer
 * added sys/param.h
 *
 * Revision 1.8  2013-05-15 00:39:03+05:30  Cprogrammer
 * added SSL encryption
 *
 * Revision 1.7  2013-03-03 23:36:52+05:30  Cprogrammer
 * read MAXHOSTNAMELEN bytes for host
 *
 * Revision 1.6  2013-02-21 22:46:05+05:30  Cprogrammer
 * use 0 as IP address for localhost
 *
 * Revision 1.5  2013-02-11 23:03:10+05:30  Cprogrammer
 * added bytes read to statusfile
 *
 * Revision 1.4  2012-09-19 11:07:08+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2012-09-18 17:08:47+05:30  Cprogrammer
 * removed syslog
 *
 * Revision 1.2  2012-09-18 14:55:24+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.1  2012-09-18 13:23:48+05:30  Cprogrammer
 * Initial revision
 *
 */
