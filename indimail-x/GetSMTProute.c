/*
 * $Id: GetSMTProute.c,v 1.6 2025-05-13 19:59:58+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include <scan.h>
#include <byte.h>
#include <open.h>
#include <getln.h>
#include <strerr.h>
#include <substdio.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: GetSMTProute.c,v 1.6 2025-05-13 19:59:58+05:30 Cprogrammer Exp mbhangui $";
#endif

int
get_smtp_qmtp_port(const char *file, const char *domain, int default_port)
{
	static stralloc line = {0};
	char            inbuf[512];
	char           *ptr;
	int             len, fd, match;
	unsigned int    i;
	struct substdio ssin;

	if ((fd = open_read(file)) == -1) {
		if (errno == ENOENT)
			return(default_port);
		else {
			strerr_warn3("GetSMTProute: ", file, ": ", &strerr_sys);
			return(-1);
		}
	}
	len = str_len(domain);
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for(;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("GetSMTProute: read: ", file, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len) /*- blank line */
				continue;
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line)) {
				strerr_warn1("GetSMTProute: out of memory", 0);
				close(fd);
				return (-1);
			}
			line.len--;
		}
		i = str_chr(line.s, '#');
		line.s[i] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;

		if (*ptr == ':') {/*- wildcard */
			close(fd);
			ptr++;
			i = str_rchr(ptr, ':');
			if (ptr[i]) {
				if (*(ptr + 1)) {
					scan_uint(ptr + 1, &i);
					return (i);
				} else
					return(default_port);
			} else
				return(default_port);
		}
		i = str_chr(ptr, ':');
		if (ptr[i]) {
			if (!byte_diff(ptr, len, domain)) {
				close(fd);
				ptr = ptr + i + 1;
				i = str_rchr(ptr, ':');
				if (ptr[i]) {
					if (*(ptr + 1)) {
						scan_uint(ptr + 1, &i);
						return (i);
					} else
						return(default_port);
				} else
					return(default_port);
			}
		}
	} /*- for (;;) */
	close(fd);
	return(default_port);
}

/*
 * qmail control  file SMTPROUTE format
 * host:relay
 * host:relay:port
 * If host is blank, all host match the line
 * If relay is blank, it means MX lookup should be done
 */
int
GetSMTProute(const char *domain)
{
	char           *sysconfdir, *controldir, *routes;
	int             default_port, relative;
	static stralloc smtproute = {0};

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative) {
		if (!stralloc_copys(&smtproute, sysconfdir) ||
				!stralloc_append(&smtproute, "/") ||
				!stralloc_cats(&smtproute, controldir) ||
				!stralloc_catb(&smtproute, "/qmtproutes", 11) ||
				!stralloc_0(&smtproute))
			return (-1);
	} else {
		if (!stralloc_copys(&smtproute, controldir) ||
				!stralloc_catb(&smtproute, "/qmtproutes", 11) ||
				!stralloc_0(&smtproute))
			return (-1);
	}
	default_port = access(smtproute.s, F_OK) ? PORT_SMTP : PORT_QMTP;
	if ((routes = env_get("ROUTES")) && *routes) {
		if (!byte_diff(routes, 4, "qmtp"))
			default_port = PORT_QMTP;
		else
		if (!byte_diff(routes, 4, "smtp"))
			default_port = PORT_SMTP;
	}
	if (relative) {
		if (!stralloc_copys(&smtproute, sysconfdir) ||
				!stralloc_append(&smtproute, "/") ||
				!stralloc_cats(&smtproute, controldir) ||
				!stralloc_catb(&smtproute, default_port == PORT_SMTP ? "/smtproutes" : "/qmtproutes", 11) ||
				!stralloc_0(&smtproute))
			return (-1);
	} else
	if (!stralloc_copys(&smtproute, controldir) ||
			!stralloc_catb(&smtproute, default_port == PORT_SMTP ? "/smtproutes" : "/qmtproutes", 11) ||
			!stralloc_0(&smtproute))
		return (-1);
	return (get_smtp_qmtp_port(smtproute.s, domain, default_port));
}
/*
 * $Log: GetSMTProute.c,v $
 * Revision 1.6  2025-05-13 19:59:58+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-03-20 10:02:16+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.3  2020-10-14 00:17:52+05:30  Cprogrammer
 * BUG: Fixed infinite loop
 *
 * Revision 1.2  2020-04-01 18:55:02+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:36:02+05:30  Cprogrammer
 * Initial revision
 *
 */
