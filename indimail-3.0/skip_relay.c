/*
 * $Log: skip_relay.c,v $
 * Revision 1.1  2019-04-18 08:36:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: skip_relay.c,v 1.1 2019-04-18 08:36:22+05:30 Cprogrammer Exp mbhangui $";
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
#endif
#include "getEnvConfig.h"

static void
die_nomem()
{
	strerr_warn1("skip_relay: out of memory", 0);
	_exit(111);
}

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
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	len = str_len(ipaddr);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("update_rules: read: ", tcp_file, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		if (line.len == 0)
			break;
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
