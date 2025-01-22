/*
 * $Id: rpclog.c,v 1.10 2025-01-22 15:58:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <time.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <signal.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/ioctl.h>
#ifdef HAVE_RPC_RPC_H
#include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif
/*- tirpc */
#ifdef HAVE_TIRPC
#ifndef HAVE_RPC_RPC_H
#include <tirpc/rpc/rpc.h>
#endif
#ifndef HAVE_RPC_TYPES_H
#include <tirpc/rpc/types.h>
#endif
#endif
#include <strerr.h>
#include <sig.h>
#include <subfd.h>
#include <qprintf.h>

#define FATAL "rpclog: fatal: "
#define WARN  "rpclog: warn: "

#define _RPCSVC_CLOSEDOWN 900

#define RPCLOG       ((u_long)0x20000001)
#define LOGVERS      ((u_long)1)
#define SEND_MESSAGE ((u_long)1)

#ifndef	lint
static char     sccsid[] = "$Id: rpclog.c,v 1.10 2025-01-22 15:58:36+05:30 Cprogrammer Exp mbhangui $";
#endif

struct CONSOLE_MSG
{
	long            globnum;
	long            r_date;
	long            r_time;
	char            machine[21];
	long            machnum;
	char            type[21];
	char            text[256];
};

static int      portmaper;    /*- Started by a port monitor ? */
static int      socket_type;  /*- Whether Stream or Datagram ? */
static int      rpcsvcdirty;  /*- Still serving ? */

bool_t          pmap_unset(unsigned long, unsigned long);
static void     rpclog_1(struct svc_req *, register SVCXPRT *);
static int     *send_message_1(char **);

int
get_options(int argc, char **argv, int *background)
{
	int             c, errflag = 0;

	while (!errflag && (c = getopt(argc, argv, "f")) != -1)
	{
		switch (c)
		{
		case 'f':
			*background = 0;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag)
		return (1);
	return (0);
}

static void
closedown(int x)
{
	if (rpcsvcdirty == 0) {
		extern fd_set   svc_fdset;
		static int      size;
		int             i, openfd;

		if (socket_type == SOCK_DGRAM)
			_exit(0);
		if (size == 0)
			size = getdtablesize();
		for (i = 0, openfd = 0; i < size && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			_exit(0);
	}
}

static void
sigterm(int x)
{
	svc_unregister(RPCLOG, LOGVERS);
	closedown(0);
	_exit(0);
}

static void
sigalarm(int x)
{
	signal(SIGALRM, closedown);
	closedown(0);
	alarm(_RPCSVC_CLOSEDOWN);
}

static void
rpclog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	typedef union
	{
	char           *send_message_1_arg;
	} myu;
        myu             argument;
	char           *result;
	bool_t(*xdr_argument) (), (*xdr_result) ();
	char           *(*local) (myu *, struct svc_req *);

	rpcsvcdirty = 1;
	switch (rqstp->rq_proc)
	{
	case NULLPROC:
		svc_sendreply(transp, (xdrproc_t) xdr_void, (char *) NULL);
		rpcsvcdirty = 0;
		return;

	case SEND_MESSAGE:
		xdr_argument = (bool_t (*)(void)) xdr_wrapstring;
		xdr_result = (bool_t (*)(void)) xdr_int;
		local = (char *(*) (myu *, struct svc_req *)) send_message_1;
		break;

	default:
		svcerr_noproc(transp);
		rpcsvcdirty = 0;
		return;
	}
	bzero((char *) &argument, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t) xdr_argument, (caddr_t) &argument)) {
		svcerr_decode(transp);
		rpcsvcdirty = 0;
		return;
	}
	result = (*local) (&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) xdr_result, result))
		svcerr_systemerr(transp);
	if (!svc_freeargs(transp, (xdrproc_t) xdr_argument, (caddr_t) &argument))
		strerr_die2x(111, FATAL, "svc_freeargs: unable to free arguments");
	rpcsvcdirty = 0;
	return;
}

static int     *
send_message_1(char **msg)
{
	static int      retval;

	retval = 1;
	if(*msg && write(1, *msg, strlen(*msg)) == -1) {
		strerr_warn2(FATAL, "write: /dev/console: ", &strerr_sys);
		retval = 0;
	}
	return &retval;
}

int
main(int argc, char **argv)
{
	register SVCXPRT *transp = (SVCXPRT *) 0;
	struct sockaddr_in saddr;
	int             i, sock, proto, background = 1, asize = sizeof(saddr);
	pid_t           pid;

	sig_termcatch(sigterm);
	if (getsockname(0, (struct sockaddr *) & saddr, (socklen_t *) &asize) == 0) {
		int             ssize = sizeof(int);

		if (saddr.sin_family != AF_INET)
			exit(1);
		/*-
		 * get socket type
		 * SO_STREAM, SO_DGRAM, etc
		 */
		if (getsockopt(0, SOL_SOCKET, SO_TYPE, (char *) &socket_type, (socklen_t *) &ssize) == -1)
			exit(1);
		sock = 0;
		portmaper = 1; /*- started by portmapper/rpcbind */
		proto = 0;
	} else {
		if (get_options(argc, argv, &background))
			strerr_die2x(100, FATAL, "Usage: rpclog [-f]");
		if (background) {
			if ((pid = fork()) < 0)
				strerr_die2sys(111, FATAL, "unable to fork");
			if (pid) /*- parent */
				_exit(0);
			setsid();
			close(0);
			close(1);
			close(2);
			if ((i = open("/dev/console", O_WRONLY)) == -1)
				strerr_die2sys(111, FATAL, "/dev/console: ");
			if (dup2(i, 1) == -1 || dup2(i, 2) == -1)
				strerr_die2sys(111, FATAL, "dup2: ");
			close(i);
		} else
			sig_intcatch(sigterm);
		sock = RPC_ANYSOCK;
		pmap_unset(RPCLOG, LOGVERS);
	}

	if (socket_type == 0 || socket_type == SOCK_DGRAM) {
		if ((transp = (SVCXPRT *) svcudp_create(sock)) == (SVCXPRT *) NULL) 
			strerr_die2sys(111, FATAL, "cannot create RPC udp service transport: ");
		if (!portmaper)
			proto = IPPROTO_UDP;
		if (!svc_register(transp, RPCLOG, LOGVERS, rpclog_1, proto))
			strerr_die2sys(111, FATAL, "unable to register (RPCLOG, LOGVERS, udp).");
		subprintf(subfdout, "RPCLOG %ld LOGVERS %ld sock %d udp %d\n", RPCLOG, LOGVERS, transp->xp_sock, transp->xp_port);
		substdio_flush(subfdout);
	}
	if (socket_type == 0 || socket_type == SOCK_STREAM) {
		if ((transp = (SVCXPRT *) svctcp_create(sock, 0, 0)) == (SVCXPRT *) NULL)
			strerr_die2sys(111, FATAL, "cannot create RPC tcp service transport: ");
		if (!portmaper)
			proto = IPPROTO_TCP;
		if (!svc_register(transp, RPCLOG, LOGVERS, rpclog_1, proto))
			strerr_die2sys(111, FATAL, "unable to register (RPCLOG, LOGVERS, tcp).");
		subprintf(subfdout, "RPCLOG %ld LOGVERS %ld sock %d tcp %d\n", RPCLOG, LOGVERS, transp->xp_sock, transp->xp_port);
		substdio_flush(subfdout);
	}
	/*xprt->xp_sock is the transport's socket descriptor, and xprt->xp_port*/
	if (transp == (SVCXPRT *) NULL)
		strerr_die2sys(111, FATAL, "cannot create RPC service transport: ");
	if (portmaper) { /*- started by portmapper */
		signal(SIGALRM, sigalarm);
		alarm(_RPCSVC_CLOSEDOWN);
	}
	svc_run();
	strerr_die2x(1, FATAL, "svc_run returned");
	/* NOTREACHED */
}

#ifndef	lint
#include <stdio.h>
void
getversion_rpclog_c()
{
	printf("%s\n", sccsid);
}
#endif
/*
 * $Log: rpclog.c,v $
 * Revision 1.10  2025-01-22 15:58:36+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.9  2023-04-14 22:31:06+05:30  Cprogrammer
 * print rpc program number and version during startup
 *
 * Revision 1.8  2023-04-14 09:43:44+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.7  2021-04-05 21:58:09+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.6  2018-08-21 19:39:23+05:30  Cprogrammer
 * fix for rpc.h on openSUSE tumbleweed
 *
 * Revision 1.5  2018-08-20 13:13:28+05:30  Cprogrammer
 * added tirpc/rpc inclusion
 *
 * Revision 1.4  2014-04-17 11:29:15+05:30  Cprogrammer
 * added conditional inclusion of stdlib.h, string.h
 *
 * Revision 1.3  2013-05-15 00:18:45+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.2  2012-09-19 11:07:26+05:30  Cprogrammer
 * rearranged functions
 *
 * Revision 1.1  2012-09-18 18:25:37+05:30  Cprogrammer
 * Initial revision
 *
 */
