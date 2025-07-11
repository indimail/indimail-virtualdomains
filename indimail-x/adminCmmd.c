/*
 * $Id: adminCmmd.c,v 1.6 2025-05-13 19:58:07+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef sun
#include <sys/filio.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <subfd.h>
#include <substdio.h>
#include <error.h>
#include <getEnvConfig.h>
#endif
#ifdef HAVE_SSL
#include "tls.h"
#endif
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: adminCmmd.c,v 1.6 2025-05-13 19:58:07+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      IOPlex(int, int);

int
adminCmmd(int sfd, int inputRead, char *cmdbuf, int len)
{
	int             i, retval, n, timeoutdata;
	char            inbuf[512], strnum[FMT_ULONG];
	char           *ptr;

	getEnvConfigInt(&timeoutdata, "TIMEOUTDATA", 120);
	if ((n = tlswrite(sfd, cmdbuf, len, timeoutdata)) != len) {
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: tlswrite: ", strnum, " bytes: ", &strerr_tls);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = tlswrite(sfd, "\n", 1, timeoutdata)) != 1) { /*- To send the command */
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: tlswrite: ", strnum, " bytes: ", &strerr_tls);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = tlswrite(sfd, "\n", 1, timeoutdata)) != 1) { /*- to make indisrvr go forward after wait() */
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: tlswrite: ", strnum, " bytes: ", &strerr_tls);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if (inputRead)
		IOPlex(sfd, timeoutdata);
	for (retval = -1;;) {
		if ((n = tlsread(sfd, inbuf, sizeof(inbuf) - 1, timeoutdata)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("adminCmmd: tlsread: ", &strerr_tls);
#ifdef HAVE_SSL
			ssl_free();
#endif
			return (-1);
		} else
		if (!n)
			break;
		inbuf[n] = 0;
		if ((ptr = str_str(inbuf, "RETURNSTATUS"))) {
			*ptr = 0;
			scan_int(inbuf + 12, &retval);
			if (ptr != inbuf) {
				i = ptr - inbuf;
				if (substdio_put(subfdoutsmall, inbuf, i) ||
						substdio_flush(subfdoutsmall))
					strerr_warn1("adminCmmd: stdout: ", &strerr_sys);
			}
			continue;
		} else
		if (substdio_put(subfdoutsmall, inbuf, n) || substdio_flush(subfdoutsmall)) {
			strerr_warn1("adminCmmd: stdout: ", &strerr_sys);
#ifdef HAVE_SSL
			ssl_free();
#endif
			return (-1);
		}
	}
#ifdef HAVE_SSL
	ssl_free();
#endif
	return (retval);
}

static int
IOPlex(int sockfd, int timeoutdata)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout, Timeout;
	unsigned int    i;
	time_t          idle_time;
	int             retval, retrycount, dataTimeout;
	char           *ptr;
	char            sockbuf[SOCKBUF + 1], strnum[FMT_ULONG];
	void            (*pstat) (int);

	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
		return (-1);
	retval = 1;
#if defined(ASM_IOCTLS_H) && defined(linux)
#include <asm/ioctls.h>
#endif
	if (ioctl(0, FIONBIO, &retval) != -1) {
		retval = 0;
		if (read(0, "", 0) == -1 || ioctl(0, FIONBIO, &retval) == -1) {
			strerr_warn1("adminCmmd: IOPlex-ioctl-FIONBIO: ", &strerr_sys);
			close(sockfd);
			signal(SIGPIPE, pstat);
			return (-1);
		}
	}  else {
		close(sockfd);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	Timeout.tv_sec = timeout.tv_sec = SELECTTIMEOUT;
	Timeout.tv_usec = timeout.tv_usec = 0;
	getEnvConfigStr(&ptr, "DATA_TIMEOUT", "1800");
	idle_time = 0;
	scan_int(ptr, &dataTimeout);
	for (retrycount = 0;;) {
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &Timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("adminCmmd: select: ", &strerr_sys);
			signal(SIGPIPE, pstat);
			return (-1);
		} else
		if (!retval) {
			idle_time += timeout.tv_sec;
			if (idle_time > dataTimeout) {
				close(sockfd);
				signal(SIGPIPE, pstat);
				strnum[i = fmt_ulong(strnum, timeout.tv_sec)] = 0;
				if (substdio_put(subfderr, "adminCmmd: select: Timeout ", 23) ||
						substdio_put(subfderr, strnum, i) ||
						substdio_flush(subfderr)) {
					strerr_warn1("adminCmmd: write-stderr: ", &strerr_sys);
					signal(SIGPIPE, pstat);
					return (-1);
				}
				return (2);
			}
			timeout.tv_sec += 2 * timeout.tv_sec;
			Timeout.tv_sec = timeout.tv_sec;
			continue;
		} else {
			timeout.tv_sec = SELECTTIMEOUT;
			idle_time = 0;
		}
		if (FD_ISSET(0, &rfds)) {
			/*- Data from Client -*/
			for (;;) {
				errno = 0;
				if ((retval = read(0, sockbuf, SOCKBUF)) == -1) {
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY) {
						usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE) {
						retval = 0;
						break;
					}
#endif
					strerr_warn1("adminCmmd: read: ", &strerr_sys);
					signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from client -*/
				break;
			/*- Write to Remote Server -*/
			if (tlswrite(sockfd, sockbuf, retval, timeoutdata) != retval) {
				strerr_warn1("adminCmmd: tlswrite: ", &strerr_tls);
				signal(SIGPIPE, pstat);
				return (-1);
			}
		} /*- if (FD_ISSET(0, &rfds)) -*/
	}
	usleep(1000);
	shutdown(sockfd, SHUT_WR);
	signal(SIGPIPE, pstat);
	return (0);
}
/*
 * $Log: adminCmmd.c,v $
 * Revision 1.6  2025-05-13 19:58:07+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.5  2023-08-08 00:34:55+05:30  Cprogrammer
 * use strerr_tls for reporting tls error
 *
 * Revision 1.4  2023-01-03 21:04:50+05:30  Cprogrammer
 * renamed ADMIN_TIMEOUT to TIMEOUTDATA
 * replaced safewrite, saferead with tlswrite, tlsread from tls library in libqmail
 *
 * Revision 1.3  2021-07-21 14:04:21+05:30  Cprogrammer
 * conditional compilation (alpine linux)
 *
 * Revision 1.2  2020-04-01 18:52:41+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.1  2019-04-18 08:39:47+05:30  Cprogrammer
 * Initial revision
 *
 */
