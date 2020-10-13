/*
 * $Log: islocalif.c,v $
 * Revision 1.4  2020-10-13 18:33:46+05:30  Cprogrammer
 * added missing alloc_free
 *
 * Revision 1.3  2020-04-01 18:56:37+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-07-04 10:06:33+05:30  Cprogrammer
 * collapsed multiple if statements
 *
 * Revision 1.1  2019-04-11 00:42:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef linux
#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif
#endif
#ifdef sun
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <alloc.h>
#include <error.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <byte.h>
#include <str.h>
#include <getEnvConfig.h>
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef	lint
static char     sccsid[] = "$Id: islocalif.c,v 1.4 2020-10-13 18:33:46+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("islocalif: out of memory", 0);
	_exit(111);
}
/*
 * RFC2553 proposes struct sockaddr_storage. This is a placeholder for all sockaddr-variant structures.
 * union
 * {
 *   struct sockaddr_storage ss;
 *   struct sockaddr_in s4;
 *   struct sockaddr_in6 s6;
 * } u;
 *
 *  - replace salocal references with u.ss
 *  - in the IPv4 branch, use u.s4 instead of the local sa variable
 *  - in the IPv6 branch, use u.s6 instead of the local sa variable  
 */

int
islocalif(char *hostptr)
{
	struct ifconf   ifc;
	struct ifreq   *ifr;
	int             fd, match, s, t, len, idx, family;
	char            inbuf[512];
	char           *sysconfdir, *controldir, *buf, *ptr;
	struct substdio ssin;
	struct sockaddr_in *sin = 0;
	static stralloc filename = {0};
	static stralloc line = { 0 };
#ifdef ENABLE_IPV6
	struct sockaddr_in6 *sin6 = 0;
#endif
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	struct sockaddr *sa;
	char            addrBuf[INET6_ADDRSTRLEN];
	char            namebuf[INET6_ADDRSTRLEN];
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	int             error, salen;
#else
	struct hostent *hp;
	char           *ipaddr;
	char            ipbuf[16];
	unsigned long   inaddr;
#endif

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) || !stralloc_catb(&filename, "/localiphost", 12) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&filename, sysconfdir) || !stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) || !stralloc_catb(&filename, "/localiphost", 12) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	if ((fd = open_read(filename.s)) == -1 && errno != error_noent)
		strerr_die3sys(111, "islocalif", filename.s, ": ");
	else
	if (fd > -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "read: ", filename.s, ": ");
		}
		if (line.len == 0)
			strerr_warn3("islocalif: ", filename.s, ": incomplete line", 0);
		else 
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		close(fd);
		if (!str_diff(hostptr, line.s))
			return (1);
	}
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	/* getaddrinfo() case.  It can handle multiple addresses. */
	byte_zero((char *) &hints, sizeof(hints));
	/* set-up hints structure */
	hints.ai_family = PF_UNSPEC;
	if ((error = getaddrinfo(hostptr, NULL, &hints, &res0)))
		strerr_die3x(111, "islocalif: getaddrinfo: ", hostptr, (char *) gai_strerror(error));
#else
	if ((inaddr = inet_addr(hostptr)) == INADDR_NONE) { /*- It's not a dotted decimal */
		if ((hp = gethostbyname(hostptr)) == NULL)
			strerr_die3x(111, "islocalif: gethostbyname: ", hostptr, (char *) hstrerror(h_errno));
		byte_copy(ipbuf, 16, inet_ntoa(*((struct in_addr *) hp->h_addr)));
		ipaddr = ipbuf;
	} else
		ipaddr = hostptr;
#endif
#ifdef ENABLE_IPV6
	s = socket(AF_INET6, SOCK_DGRAM, 0);
	if (s == -1 && (errno == EINVAL || errno == EAFNOSUPPORT))
		s = socket(AF_INET, SOCK_STREAM, 0);
#else
	s = socket(AF_INET, SOCK_DGRAM, 0);
#endif
	if (s == -1) {
#ifdef ENABLE_IPV6
		freeaddrinfo(res0);
#endif
		return (-1);
	}
	len = 8192;
	for (t = 0, buf = (char *) 0;;) {
		if (!alloc_re((char *) &buf, t, len * sizeof(char)))
			die_nomem();
		ifc.ifc_buf = buf;
		ifc.ifc_len = len;
		if (ioctl(s, SIOCGIFCONF, &ifc) >= 0) {/*- > is for System V */
			if (ifc.ifc_len + sizeof(*ifr) + 64 < len) {
				/*- what a stupid interface */
				*(buf + ifc.ifc_len) = 0;
				break;
			}
		}
		if (len > 200000) {
			close(s);
			alloc_free(buf);
#ifdef ENABLE_IPV6
			freeaddrinfo(res0);
#endif
			return (-1);
		}
		t = len;
		len *= 2;
	}
	ifr = ifc.ifc_req;
	for (idx = ifc.ifc_len / sizeof(struct ifreq); --idx >= 0; ifr++) {
		family = ifr->ifr_addr.sa_family;
		if (family == AF_INET)
			sin = (struct sockaddr_in *) &ifr->ifr_addr;
#ifdef ENABLE_IPV6
		else
		if (family == AF_INET6)
			sin6 = (struct sockaddr_in6 *) &ifr->ifr_addr;
#endif
		else
			continue;
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
#ifdef ENABLE_IPV6
		if (!(ptr = (char *) inet_ntop(family, family == AF_INET ? (void *) &sin->sin_addr : (void *) &sin6->sin6_addr,
			addrBuf, INET6_ADDRSTRLEN)))
#else
		if (!(ptr = (char *) inet_ntop(AF_INET, (void *) &sin->sin_addr, addrBuf, INET6_ADDRSTRLEN)))
#endif
			continue;
		if (ioctl(s, SIOCGIFFLAGS, (char *) ifr))
			continue;
		/* Skip boring cases */
		if ((ifr->ifr_flags & IFF_UP) == 0)
			continue;
		/*
		if ((ifr->ifr_flags & (IFF_BROADCAST | IFF_POINTOPOINT)) == 0)
			continue;*/
		for (res = res0; res; res = res->ai_next) {
			sa = res->ai_addr;
			salen = res->ai_addrlen;
			if (getnameinfo(sa, salen, namebuf, sizeof(namebuf), 0, 0, NI_NUMERICHOST)) {
				strerr_warn1("islocalif: getnameinfo: ", &strerr_sys);
				continue;
			}
			if (res->ai_flags | AI_NUMERICHOST) {
				if (!str_diffn(namebuf, ptr, INET6_ADDRSTRLEN)) {
					close(s);
					alloc_free(buf);
					freeaddrinfo(res0);
					return (1);
				}
			}
		}
#else
		if (!(ptr = inet_ntoa(sin->sin_addr)))
			continue;
		if (!str_diffn(ipaddr, ptr, INET_ADDRSTRLEN)) {
			close(s);
			alloc_free(buf);
			return (1);
		}
#endif /*- #ifdef HAVE_STRUCT_SOCKADDR_STORAGE */
	} /*- for (idx = ifc.ifc_len / sizeof(struct ifreq); --idx >= 0; ifr++) */
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	freeaddrinfo(res0);
#endif
	alloc_free(buf);
	close(s);
	return (0);
}
