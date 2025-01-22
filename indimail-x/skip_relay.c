/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: skip_relay.c,v 1.4 2023-03-20 10:18:10+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#include <stralloc.h>
#include <getEnvConfig.h>
#endif

int
skip_relay(char *ipaddr)
{
	static stralloc line = {0};
	char           *ptr, *tcp_file;
	int             fd, match, i, len;
	char            inbuf[4096];
	struct substdio ssin;

	getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
	if ((fd = open_read(tcp_file)) == -1) {
		if (errno == error_noent)
			return (0);
		strerr_warn3("skip_relay: ", tcp_file, ": ", &strerr_sys);
		return (1);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	len = str_len(ipaddr);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("update_rules: read: ", tcp_file, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len) {
				strerr_warn2("update_rules: incomplete line: ", tcp_file, 0);
				continue;
			}
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line)) {
				strerr_warn1("update_rules: out of memory", 0);
				close(fd);
				return (-1);
			}
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!str_str(ptr, "allow") || !str_str(ptr, "RELAYCLIENT"))
			continue;
		i = str_chr(ptr, ':');
		if (ptr[i]) {
			*(ptr + i) = 0;
			if (!str_diffn(ptr, ipaddr, len)) {
				close(fd);
				return (1);
			}
		}
	}
	close(fd);
	return (0);
}
#endif
/*
 * $Log: skip_relay.c,v $
 * Revision 1.4  2023-03-20 10:18:10+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.3  2020-10-14 00:19:48+05:30  Cprogrammer
 * new logic for terminating a line
 *
 * Revision 1.2  2020-04-01 18:57:52+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:36:22+05:30  Cprogrammer
 * Initial revision
 *
 */
