/*
 * $Id: $
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
#include <getEnvConfig.h>
#endif
#include "get_assign.h"

#ifndef	lint
static char     sccsid[] = "$Id: is_alias_domain.c,v 1.5 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("is_alias_domain: out of memory", 0);
	_exit(111);
}

int
is_alias_domain(const char *domain)
{
	int             fd, match, t;
	char            inbuf[512];
	char           *ptr;
	static stralloc dir = {0}, line = {0}, tmp = {0};
	struct stat     statbuf;
	struct substdio ssin;

	if (get_assign(domain, &dir, NULL, NULL) == 0)
		return (0);
	if (lstat(dir.s, &statbuf)) {
		if (errno != error_noent) {
			strerr_warn3("is_alias_domain: lstat: ", dir.s, ": ", &strerr_sys);
			return -1;
		}
		return (0);
	}
	if (S_ISLNK(statbuf.st_mode)) { /*- we sould do readlink and find out */
		if ((t = readlink(dir.s, inbuf, sizeof(inbuf))) == -1) {
			strerr_warn3("is_alias_domain: readlink: ", dir.s, ": ", &strerr_sys);
			return -1;
		}
		if (t == sizeof(inbuf)) {
			errno = ENAMETOOLONG;
			strerr_warn3("is_alias_domain: readlink: ", dir.s, ": ", &strerr_sys);
			return -1;
		}
		inbuf[t] = 0;
		return (access(inbuf, F_OK) ? 0 : 1);
	}
	if (!stralloc_copy(&tmp, &dir) || !stralloc_catb(&tmp, "/.aliasdomains", 14) ||
			!stralloc_0(&tmp))
		die_nomem();
	if ((fd = open_read(tmp.s)) == -1) {
		if (errno != error_noent) {
			strerr_warn3("is_alias_domain: open: ", tmp.s, ": ", &strerr_sys);
			return -1;
		}
		return (0);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die3sys(111, "is_alias_domain: read: ", tmp.s, ": ");
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
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
/*
 * $Log: is_alias_domain.c,v $
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-03-25 17:49:16+05:30  Cprogrammer
 * bug fix - fixed using resolved link
 *
 * Revision 1.3  2023-03-20 10:08:07+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2021-09-11 13:26:14+05:30  Cprogrammer
 * use getEnvConfig for domain directory
 * on system error, return -1 instead of exit
 *
 * Revision 1.1  2019-04-18 08:25:36+05:30  Cprogrammer
 * Initial revision
 *
 */
