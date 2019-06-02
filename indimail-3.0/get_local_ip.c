/*
 * $Log: get_local_ip.c,v $
 * Revision 1.1  2019-04-14 18:32:24+05:30  Cprogrammer
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <open.h>
#include <getln.h>
#include <strerr.h>
#include <byte.h>
#include <str.h>
#include <substdio.h>
#include <error.h>
#endif
#include "getEnvConfig.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_local_ip.c,v 1.1 2019-04-14 18:32:24+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc hostbuf = { 0 };
static stralloc TmpBuf = { 0 };
static stralloc line = { 0 };
static char     inbuf[512];

static void
die_nomem()
{
	strerr_warn1("get_local_ip: out of memory", 0);
	_exit(111);
}

char           *
get_local_ip(int family)
{
	char           *sysconfdir, *controldir;
	int             t, fd, match;
	struct substdio ssin;
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	struct sockaddr *sa;
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	int             error, salen;
#else
	struct hostent *host_data;
#endif

	if (hostbuf.len)
		return (hostbuf.s);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&TmpBuf, controldir))
			die_nomem();
		else
		if (!stralloc_catb(&TmpBuf, "/localiphost", 12))
			die_nomem();
		else
		if (!stralloc_0(&TmpBuf))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&TmpBuf, sysconfdir))
			die_nomem();
		else
		if (!stralloc_catb(&TmpBuf, "/", 1))
			die_nomem();
		else
		if (!stralloc_cats(&TmpBuf, controldir))
			die_nomem();
		else
		if (!stralloc_catb(&TmpBuf, "/localiphost", 12))
			die_nomem();
	}
	if ((fd = open_read(TmpBuf.s)) == -1 && errno != error_noent) {
		strerr_die3sys(111, "get_local_ip: ", TmpBuf.s, ": ");
	} else
	if (fd > -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "get_local_ip: read: ", TmpBuf.s, ": ");
		}
		if (line.len == 0)
			strerr_warn3("get_local_ip", TmpBuf.s, "incomplete line", 0);
		else
		if (match) {
			line.len--;
			if (line.len == 0)
				strerr_warn3("get_local_ip", TmpBuf.s, "incomplete line", 0);
			line.s[line.len] = 0; /*- remove newline */
		}
		close(fd);
		return (line.s);
	}
	if (!stralloc_ready(&TmpBuf, 64))
		die_nomem();
	if (gethostname(TmpBuf.s, 64))
		strerr_die1sys(111, "get_local_ip: ");
	if (!stralloc_0(&TmpBuf))
		die_nomem();
#ifdef ENABLE_IPV6
	byte_zero((char *) &hints, sizeof(hints));
	/* set-up hints structure */
	hints.ai_family = family > 0 ? family : PF_UNSPEC; /*- Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /*- Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0; /*- Any protocol */
	if ((error = getaddrinfo(TmpBuf.s, 0, &hints, &res0))) {
		strerr_die2x(111, "get_local_ip: getaddrinfo: ", (char *) gai_strerror(errno));
	} else {
		for (res = res0; res; res = res->ai_next) {
			sa = res->ai_addr;
			salen = res->ai_addrlen;
			/* do what you want */
			/* getnameinfo() case. NI_NUMERICHOST avoids DNS lookup. */
			if (!stralloc_ready(&hostbuf, 64))
				die_nomem();
			if ((error = getnameinfo(sa, salen, hostbuf.s, hostbuf.len, 0, 0, NI_NUMERICHOST))) {
				freeaddrinfo(res0);
				strerr_die2x(111, "get_local_ip: getnameinfo: ", (char *) gai_strerror(errno));
			}
			if (res->ai_flags | AI_NUMERICHOST) {
				freeaddrinfo(res0);
				return (hostbuf.s);
			}
		}
		freeaddrinfo(res0);
		return ((char *) 0);
	}
#else
	if (!(host_data = gethostbyname(TmpBuf.s)))
		return ((char *) 0);
	if (!stralloc_copys(&hostbuf, inet_ntoa(*((struct in_addr *) host_data->h_addr_list[0]))))
		die_nomem();
#endif /*- ENABLE_IPV6 */
	return (hostbuf.s);
}
