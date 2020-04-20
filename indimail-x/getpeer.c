/*
 * $Log: getpeer.c,v $
 * Revision 1.1  2019-04-18 08:18:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef sun
#include <sys/filio.h>
#include <stropts.h>
#endif
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifndef	lint
static char     sccsid[] = "$Id: getpeer.c,v 1.1 2019-04-18 08:18:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

/*-
 * Function: GetIpaddr
 *
 * Description:
 *     Returns pointers to the inet address string and the host string
 *     for the system that contacted inetd. Special code handles both
 *     tcp and udp connections.
-*/
char           *
GetPeerIPaddr()
{
#ifdef ENABLE_IPV6
	static char     addrBuf[INET6_ADDRSTRLEN];
	struct sockaddr_storage sa;
#else
	struct sockaddr sa;
#endif
	int             length = sizeof(sa);
	int             on;
	char            buf[256];

	/*- Translate the internet address to a string */
	if (getpeername(0, (struct sockaddr *) &sa, (socklen_t *) &length) == -1) {
		/*- If getpeername() fails we assume a udp socket...  */
		switch (errno)
		{
		case ENOTSOCK: /*- stdin is not a socket -*/
			return ("127.0.0.1");
		case ENOTCONN: /*- assume UDP request -*/
			/*- Set I/O to non-blocking */
			on = 1;
			if (ioctl(0, FIONBIO, &on) < 0)
				return ("?.?.?.?");
			on = 0;
			if (recvfrom(0, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) &sa, (socklen_t *) &length) < 0) {
				(void) ioctl(0, FIONBIO, &on);
				return ("?.?.?.?");
			}
			(void) ioctl(0, FIONBIO, &on);
			break;
		default:
			return ("?.?.?.?");
		}
	}
	/*
	 * Now that we have the remote host address, look up the remote host
	 * name. Use the address if name lookup fails. At present, we can
	 * only handle names or addresses that belong to the AF_INET and
	 * AF_CCITT family.
	 */
#ifdef ENABLE_IPV6
	if (((struct sockaddr *) &sa)->sa_family == AF_INET) {
		return ((char *) inet_ntop(AF_INET, (void *) &((struct sockaddr_in *) &sa)->sin_addr,
			addrBuf, INET_ADDRSTRLEN));
	} else
	if (((struct sockaddr *)&sa)->sa_family == AF_INET6) {
		return ((char *) inet_ntop(AF_INET6, (void *) &((struct sockaddr_in6 *) &sa)->sin6_addr,
			addrBuf, INET6_ADDRSTRLEN));
	} else
	if (((struct sockaddr *) &sa)->sa_family == AF_UNSPEC)
		return ("127.0.0.1");
#else
	if (sa.sa_family == AF_INET)
		return (inet_ntoa(((struct sockaddr_in *) &sa)->sin_addr));
	else
	if (sa.sa_family == AF_UNSPEC)
		return ("127.0.0.1");
#endif
	return ("0.0.0.0");
}

char           *
getremoteip()
{
	char           *tcpremoteip;
	struct sockaddr sa;
	int             i, status, dummy;
#ifdef ENABLE_IPV6
	static char     addrBuf[INET6_ADDRSTRLEN];
#endif

	dummy = sizeof(sa);
	for(tcpremoteip = (char *) 0, errno = i = 0;i < 128;i++) {
		if (!(status = getpeername(i, (struct sockaddr *) &sa, (socklen_t *) &dummy))) {
#ifdef ENABLE_IPV6
			if (sa.sa_family == AF_INET)
				tcpremoteip = (char *) inet_ntop(AF_INET, (void *) &((struct sockaddr_in *) &sa)->sin_addr,
					addrBuf, INET_ADDRSTRLEN);
			else
			if (sa.sa_family == AF_INET6)
				tcpremoteip = (char *) inet_ntop(AF_INET6, (void *) &((struct sockaddr_in6 *) &sa)->sin6_addr,
					addrBuf, INET6_ADDRSTRLEN);
#else
			if (sa.sa_family == AF_INET)
				tcpremoteip = inet_ntoa(((struct sockaddr_in *) &sa)->sin_addr);
#endif
			else
			if (sa.sa_family == AF_UNSPEC)
				return ("127.0.0.1");
			else
				return ("0.0.0.0");
			break;
		}
		if (errno == EBADF)
			return ((char *) 0);
	}
	return (tcpremoteip);
}
