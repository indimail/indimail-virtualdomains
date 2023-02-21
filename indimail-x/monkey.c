/*
 * $Log: monkey.c,v $
 * Revision 1.5  2021-07-21 14:05:05+05:30  Cprogrammer
 * conditional compilation (alpine linux)
 *
 * Revision 1.4  2020-10-01 18:26:49+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.3  2020-04-01 18:57:19+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-22 23:14:13+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-18 08:36:08+05:30  Cprogrammer
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
#include <sys/systeminfo.h>
#endif
#ifdef linux
#include <sys/param.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <str.h>
#include <scan.h>
#include <fmt.h>
#include <scan.h>
#include <error.h>
#include <substdio.h>
#include <subfd.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"
#include "tcpopen.h"
#include "isnum.h"
#include "sockwrite.h"

#ifndef	lint
static char     sccsid[] = "$Id: monkey.c,v 1.5 2021-07-21 14:05:05+05:30 Cprogrammer Exp mbhangui $";
#endif

int
monkey(char *host, char *servicename, char *startbuf, int skip_nl)
{
	int             i, sockfd, retval, retrycount, gotNl, dataTimeout;
	fd_set          rfds;	/*- File descriptor mask for select -*/
	char            sockbuf[SOCKBUF + 1];
#ifdef _SC_HOST_NAME_MAX
	char            localhost[_SC_HOST_NAME_MAX];
#else
	char            localhost[MAXHOSTNAMELEN];
#endif /*- SC_HOST_NAME_MAX */
	char            strnum[FMT_ULONG];
	struct timeval  timeout;
	time_t          idle_time, last_timeout;
	char           *ptr, *cptr;
	void            (*pstat) ();

#ifdef SOLARIS
#ifdef _SC_HOST_NAME_MAX
	if (sysinfo(SI_HOSTNAME, localhost, _SC_HOST_NAME_MAX) > _SC_HOST_NAME_MAX)
#else
	if (sysinfo(SI_HOSTNAME, localhost, MAXHOSTNAMELEN) > MAXHOSTNAMELEN)
#endif /*- SC_HOST_NAME_MAX */
#else
#ifdef _SC_HOST_NAME_MAX
	if (gethostname(localhost, _SC_HOST_NAME_MAX))
#else
	if (gethostname(localhost, MAXHOSTNAMELEN))
#endif /*- SC_HOST_NAME_MAX */
#endif
		return (-1);
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
		return (-1);
	if (!str_diff(host, localhost))
		ptr = "localhost";
	else
		ptr = host;
	/*- IP connection service provided by inetd -*/
	if (isnum(servicename)) {
		scan_int(servicename, &i);
		sockfd = tcpopen(ptr, (char *) 0, i);
	} else
		sockfd = tcpopen(ptr, servicename, 0);
	if (sockfd == -1) {
		strerr_warn5("monkey: ", ptr, ":", servicename, ": ", &strerr_sys);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	retval = 1;
#if defined(ASM_IOCTLS_H) && defined(linux)
#include <asm/ioctls.h>
#endif
	if (ioctl(0, FIONBIO, &retval) != -1) {
		retval = 0;
		if (read(0, "", 0) == -1 || ioctl(0, FIONBIO, &retval) == -1) {
			strerr_warn1("monkey: ioctl-FIONBIO: ", &strerr_sys);
			close(sockfd);
			signal(SIGPIPE, pstat);
			return (-1);
		}
	}  else {
		close(sockfd);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	if (startbuf && *startbuf &&
			(sockwrite(sockfd, startbuf, str_len(startbuf)) == -1 ||
			 sockwrite(sockfd, "", 0) == -1))
	{
		strerr_warn1("monkey: sockwrite-startbuf: ", &strerr_sys);
		close(sockfd);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	getEnvConfigStr(&ptr, "DATA_TIMEOUT", "1800");
	scan_int(ptr, &dataTimeout);
	timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
	timeout.tv_usec = 0;
	idle_time = 0; /*- for how long connection is idle */
	last_timeout = timeout.tv_sec;
	for (gotNl = retrycount = 0;;) {
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(sockfd, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("monkey: select: ", &strerr_sys);
			close(sockfd);
			signal(SIGPIPE, pstat);
			return (-1);
		} else
		if (!retval) { /*- timeout occurred */
			idle_time += last_timeout - timeout.tv_sec;
			if (idle_time >= dataTimeout) {
				close(sockfd);
				signal(SIGPIPE, pstat);
				strnum[i = fmt_ulong(strnum, timeout.tv_sec)] = 0;
				if (substdio_put(subfderr, "monkey-select: Timeout ", 23) ||
						substdio_put(subfderr, strnum, i) ||
						substdio_flush(subfderr)) {
					signal(SIGPIPE, pstat);
					return (-1);
				}
				return (2);
			}
			last_timeout += 2 * last_timeout;
			timeout.tv_sec = last_timeout;
			continue;
		} else {
			last_timeout = timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
			timeout.tv_usec = idle_time = 0;
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
						(void) usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE)
					{
						retval = 0;
						break;
					}
#endif
					strerr_warn1("monkey: read: ", &strerr_sys);
					close(sockfd);
					signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from client -*/
				break;
			/*- Write to Remote Server -*/
			if (sockwrite(sockfd, sockbuf, retval) != retval) {
				strerr_warn1("monkey: sockwrie: ", &strerr_sys);
				(void) close(sockfd);
				(void) signal(SIGPIPE, pstat);
				return (-1);
			}
		} /*- if (FD_ISSET(0, &rfds)) -*/
		if (FD_ISSET(sockfd, &rfds)) {
			/*- Data from Remote Server -*/
			for (;;) {
				errno = 0;
				if ((retval = read(sockfd, sockbuf, SOCKBUF)) < 0) {
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY) {
						(void) usleep(1000);
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
					(void) close(sockfd);
					(void) signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from server -*/
				break;
			/*- Write data to Client -*/
			if (gotNl < skip_nl) {
				sockbuf[retval] = 0;
				for (ptr = sockbuf, i = str_chr(sockbuf, '\n'); *ptr && sockbuf[i];) {
					i = str_chr(ptr, '\n');
					if (ptr[i]) {
						cptr = ptr +i;
						gotNl++;
						if (gotNl == skip_nl) {
							if ((retval - (cptr - sockbuf + 1)) > 0)
								if (write(1, cptr + 1, retval - (cptr - sockbuf + 1)) == -1)
									;
						}
						if (cptr < sockbuf + retval)
							ptr = cptr + 1;
					}
				}
				continue;
			}
			if (sockwrite(1, sockbuf, retval) != retval) {
				strerr_warn1("monkey: sockwrite: ", &strerr_sys);
				close(sockfd);
				signal(SIGPIPE, pstat);
				return (-1);
			}
		}	/*- end of FD_ISSET(sockfd, &rfds) -*/
	}	/*- end of for -*/
	(void) usleep(1000);
	(void) close(sockfd);
	(void) signal(SIGPIPE, pstat);
	return (0);
}
