/*
 * $Log: udpopen.c,v $
 * Revision 1.1  2019-04-22 23:26:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <subfd.h>
#include <error.h>
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#include <byte.h>
#endif
#include "isnum.h"

#ifndef	lint
static char     sccsid[] = "$Id: udpopen.c,v 1.1 2019-04-22 23:26:09+05:30 Cprogrammer Exp mbhangui $";
#endif

int
udpopen(char *rhost, char *servicename)
{
	int             fd, retval;
	struct servent *sp;	/*- pointer to service information */
#ifdef ENABLE_IPV6
	struct addrinfo hints = {0}, *res = 0, *res0 = 0;
	char            serv[FMT_ULONG];
#else
	struct hostent *hp;	/*- pointer to host info for nameserver host */
	struct sockaddr_in tcp_srv_addr;
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
	int             i;
#endif
#endif

#ifdef ENABLE_IPV6
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (!servicename) {
		errno = EINVAL;
		return (-1);
	} 
	if (isnum(servicename))
		byte_copy(serv, FMT_ULONG, servicename);
	else {
		if ((sp = getservbyname(servicename, "udp")) == NULL) {
			errno = EINVAL;
			return (-1);
		}
		serv[fmt_ulong(serv, htons(sp->s_port))] = 0;
	}
	if ((retval = getaddrinfo(rhost, serv, &hints, &res0)))
		strerr_die7x(111, "udpopen", "getadrinfo: ", rhost, ": ", serv, ":", (char *) gai_strerror(retval));
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next) {
		if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
			continue; /*- Try the next address record in the list. */
		if (!(retval = connect(fd, res->ai_addr, res->ai_addrlen)))
			break;
		retval = errno;
		close(fd);
		fd = -1;
		errno = retval;
	} /*- for (res = res0; res; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	/*- clear out address structures */
	byte_zero((char *) &tcp_srv_addr, sizeof(struct sockaddr_in));
	tcp_srv_addr.sin_family = AF_INET;
	if (isnum(servicename)) {
		scan_int(servicename, &i);
		tcp_srv_addr.sin_port = htons(i);
	} else {
		if ((sp = getservbyname(servicename, "udp")) == NULL) {
			errno = EINVAL;
			return (-1);
		}
		tcp_srv_addr.sin_port = htons(sp->s_port);
	}
	if ((inaddr = inet_addr(rhost)) != INADDR_NONE) /*- It's a dotted decimal */
		byte_copy((char *) &tcp_srv_addr.sin_addr, sizeof(inaddr), (char *) &inaddr);
	else
	if ((hp = gethostbyname(rhost)) == NULL)
		strerr_die5x(111, "tcpopen", "gethostbyname: ", rhost, ": ", (char *) hstrerror(h_errno));
	else
		byte_copy((char *) &tcp_srv_addr.sin_addr, hp->h_length, hp->h_addr);
	/*- Create the socket. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);
	if ((retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))) == -1)
	{
		retval = errno;
		close(fd);
		errno = retval;
		return (-1);
	}
#endif
	return(fd);
}
