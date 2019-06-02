/*
 * $Log: is_alias_domain.c,v $
 * Revision 1.1  2019-04-18 08:25:36+05:30  Cprogrammer
 * Initial revision
 *
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <error.h>
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#endif
#include "get_assign.h"

#ifndef	lint
static char     sccsid[] = "$Id: is_alias_domain.c,v 1.1 2019-04-18 08:25:36+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("is_alias_domain: out of memory", 0);
	_exit(111);
}

int
is_alias_domain(char *domain)
{
	int             fd, match, t;
	char            inbuf[512];
	char           *ptr;
	static stralloc dir = {0}, line = {0}, tmp = {0};
	struct stat     statbuf;
	struct substdio ssin;

	if (get_assign(domain, &dir, NULL, NULL) == 0)
		return (0);
	if (lstat(dir.s, &statbuf))
		strerr_die3sys(111, "is_alias_domain: lstat: ", dir.s, ": ");
	if (S_ISLNK(statbuf.st_mode))
		return (1);
	if (!stralloc_copy(&tmp, &dir) || !stralloc_catb(&tmp, "/.aliasdomains", 14) ||
			!stralloc_0(&tmp))
		die_nomem();
	if ((fd = open_read(tmp.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "is_alias_domain: ", tmp.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "is_alias_domain: read: ", tmp.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!(str_diff(ptr, domain))) {
			close(fd);
			return (1);
		}
	}
	close(fd);
	return (0);
}
