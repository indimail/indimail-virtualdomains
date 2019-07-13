/*
 * $Log: adminCmmd.c,v $
 * Revision 1.1  2019-04-18 08:39:47+05:30  Cprogrammer
 * Initial revision
 *
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
#endif
#ifdef HAVE_SSL
#include "tls.h"
#endif
#include "getEnvConfig.h"
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: adminCmmd.c,v 1.1 2019-04-18 08:39:47+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      IOPlex(int, int);

int
adminCmmd(int sfd, int inputRead, char *cmdbuf, int len)
{
	int             i, retval, n, admin_timeout;
	char            inbuf[512], strnum[FMT_ULONG];
	char           *ptr;

	getEnvConfigInt(&admin_timeout, "ADMIN_TIMEOUT", 120);
	if ((n = safewrite(sfd, cmdbuf, len, admin_timeout)) != len) {
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: safewrite: ", strnum, " bytes: ", &strerr_sys);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = safewrite(sfd, "\n", 1, admin_timeout)) != 1) { /*- To send the command */
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: safewrite: ", strnum, " bytes: ", &strerr_sys);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = safewrite(sfd, "\n", 1, admin_timeout)) != 1) { /*- to make indisrvr go forward after wait() */
		strnum[fmt_int(strnum, n)] = 0;
		strerr_warn3("adminCmmd: safewrite: ", strnum, " bytes: ", &strerr_sys);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if (inputRead)
		IOPlex(sfd, admin_timeout);
	for (retval = -1;;) {
		if ((n = saferead(sfd, inbuf, sizeof(inbuf) - 1, admin_timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("adminCmmd: saferead: ", &strerr_sys);
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
IOPlex(int sockfd, int admin_timeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout, Timeout;
	unsigned int    i;
	time_t          idle_time;
	int             retval, retrycount, dataTimeout;
	char           *ptr;
	char            sockbuf[SOCKBUF + 1], strnum[FMT_ULONG];
	void            (*pstat) ();

	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
		return (-1);
	retval = 1;
#ifdef linux
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
			if (safewrite(sockfd, sockbuf, retval, admin_timeout) != retval) {
				strerr_warn1("adminCmmd: safewrite: ", &strerr_sys);
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
