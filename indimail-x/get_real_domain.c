/*
 * $Log: get_real_domain.c,v $
 * Revision 1.6  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-03-25 16:33:11+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.4  2023-03-20 10:02:10+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.3  2021-09-11 13:36:34+05:30  Cprogrammer
 * corrected wrong variable used for domain directory
 *
 * Revision 1.2  2020-04-01 18:54:59+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 15:41:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <error.h>
#include <env.h>
#include <open.h>
#include <strerr.h>
#include <substdio.h>
#include <getln.h>
#include <getEnvConfig.h>
#endif
#include "get_assign.h"
#include "is_distributed_domain.h"
#include "sql_get_realdomain.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_real_domain.c,v 1.6 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
static char    *sysconfdir, *controldir;
static stralloc filename = {0}, line = {0}, prevDomainVal = {0},
				domval = {0};
#endif
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static void
die_nomem()
{
	strerr_warn1("get_real_domain: out of memory", 0);
	_exit(111);
}

#ifdef CLUSTERED_SITE
static char *
is_in_rcpthosts(const char *domain)
{
	int             fd, match;
	char            inbuf[512];
	char           *ptr;
	struct substdio ssin;

	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/rcpthosts", 10) ||
				!stralloc_0(&filename))
			die_nomem();
	} else 
	if (!stralloc_copys(&filename, sysconfdir) ||
			!stralloc_append(&filename, "/") ||
			!stralloc_cats(&filename, controldir) ||
			!stralloc_catb(&filename, "/rcpthosts", 10) ||
			!stralloc_0(&filename))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "get_real_domain: ", filename.s, ": ");
	} else {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				strerr_die3sys(111, "get_real_domain: read: ", filename.s, ": ");
			if (!line.len)
				break;
			if (match) {
				line.len--;
				if (!line.len) {
					strerr_warn3("get_real_domain: ", filename.s, ": incomplete line", 0);
					continue;
				}
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
			if (!str_diff(domain, ptr)) {
				if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
					die_nomem();
				prevDomainVal.len--;
				if (!stralloc_copys(&domval, domain) || !stralloc_0(&domval))
					die_nomem();
				domval.len--;
				return (domval.s);
			}
		}
		close(fd);
	}
	if (*controldir == '/') {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/morercpthosts", 14) ||
				!stralloc_0(&filename))
			die_nomem();
	} else
	if (!stralloc_copys(&filename, sysconfdir) ||
			!stralloc_append(&filename, "/") ||
			!stralloc_cats(&filename, controldir) ||
			!stralloc_catb(&filename, "/morercpthosts", 14) ||
			!stralloc_0(&filename))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "get_real_domain: ", filename.s, ": ");
		return ((char *) 0);
	} else {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				strerr_die3sys(111, "get_real_domain: read: ", filename.s, ": ");
			if (!line.len)
				break;
			if (match) {
				line.len--;
				if (!line.len) {
					strerr_warn3("get_real_domain", filename.s, "incomplete line", 0);
					continue;
				}
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
			if (!str_diff(domain, ptr)) {
				if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
					die_nomem();
				prevDomainVal.len--;
				if (!stralloc_copys(&domval, domain) || !stralloc_0(&domval))
					die_nomem();
				domval.len--;
				return (domval.s);
			}
		}
		close(fd);
	}
	return ((char *) 0);
}
#endif

static char *
is_in_env(const char *domain, const char *envstr)
{
	char           *ptr, *cptr;
	int             len;

	if ((ptr = env_get(envstr))) {
		len = str_len(domain);
		for (cptr = ptr; *cptr; cptr++) {
			if (*cptr == ':') {
				*cptr = 0;
				if (!str_diffn(domain, ptr, len + 1)) {
					if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
						die_nomem();
					prevDomainVal.len--;
					if (!stralloc_copys(&domval, domain) || !stralloc_0(&domval))
						die_nomem();
					domval.len--;
					return (domval.s);
				}
				ptr = cptr + 1;
				*cptr = ':';
			}
			if (*ptr && !str_diffn(domain, ptr, len + 1)) {
				if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
					die_nomem();
				prevDomainVal.len--;
				if (!stralloc_copys(&domval, domain) || !stralloc_0(&domval))
					die_nomem();
				domval.len--;
				return (domval.s);
			}
		}
	}
	return ((char *) 0);
}

const char     *
get_real_domain(const char *domain)
{
	static stralloc dir = {0};
	char            Dir[1024];
	char           *ptr;
	struct stat     statbuf;
	int             len, match;
	uid_t           uid;
	gid_t           gid;
#ifdef CLUSTERED_SITE
	int             ret;
#endif

	if (!domain || !*domain)
		return ((char *) 0);
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE")) {
		if (prevDomainVal.len && domval.len && !str_diffn(domain, prevDomainVal.s, prevDomainVal.len + 1))
			return (domval.s);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	/*
	 * e.g. indimail.org:yahoo.com:hotmail.com
	 */
	if ((ptr = is_in_env(domain, "REAL_DOMAINS")))
		return ptr;
	if ((ptr = is_in_env(domain, "ALIAS_DOMAINS")))
		return ptr;
	if (!get_assign(domain, &dir, &uid, &gid))
#ifdef CLUSTERED_SITE
	{
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/') {
			if (!stralloc_copys(&filename, controldir) ||
					!stralloc_catb(&filename, "/host.cntrl", 11) ||
					!stralloc_0(&filename))
				die_nomem();
		} else
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/host.cntrl", 11) ||
				!stralloc_0(&filename))
			die_nomem();
		if (!access(filename.s, F_OK)) {
			if ((ret = is_distributed_domain(domain)) == -1 || ret == 1)
				return ((char *) 0);
			if (!(ptr = sql_get_realdomain(domain))) /*- check aliasdomain table on central db */
				return ((char *) 0);
			else {
				if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
					die_nomem();
				prevDomainVal.len--;
				if (!stralloc_copys(&domval, ptr) || !stralloc_0(&domval))
					die_nomem();
				domval.len--;
				return (domval.s);
			}
		} else
			return (is_in_rcpthosts(domain)); /*- a domain in rcpthosts not under control of indimail */
	}
#else
		return ((char *) 0);
#endif
	if (lstat(dir.s, &statbuf)) {
		if (errno != ENOENT) {
			strerr_die4sys(111, "get_real_domain: ", "lstat: ", dir.s, ": ");
			userNotFound = 0;
		} else
			userNotFound = 1;
		return ((char *) 0);
	}
	if (S_ISLNK(statbuf.st_mode)) {
		if ((len = readlink(dir.s, Dir, sizeof(Dir))) == -1)
			return ((char *) 0);
		if (len < sizeof(Dir))
			Dir[len] = 0;
		else {
			errno = ENAMETOOLONG;
			return ((char *) 0);
		}
		match = str_rchr(Dir, '/');
		if (Dir[match])
			ptr = Dir + match + 1;
		else
			ptr = Dir;
		if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
			die_nomem();
		prevDomainVal.len--;
		if (!stralloc_copys(&domval, ptr) || !stralloc_0(&domval))
			die_nomem();
		domval.len--;
		return (domval.s);
	}
	if (!stralloc_copys(&prevDomainVal, domain) || !stralloc_0(&prevDomainVal))
		die_nomem();
	prevDomainVal.len--;
	if (!stralloc_copys(&domval, domain) || !stralloc_0(&domval))
		die_nomem();
	domval.len--;
	return (domain);
}

#ifdef QUERY_CACHE
void
get_real_domain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
