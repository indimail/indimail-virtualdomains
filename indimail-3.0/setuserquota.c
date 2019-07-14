/*
 * $Log: setuserquota.c,v $
 * Revision 1.1  2019-04-14 18:28:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#include <fmt.h>
#endif
#include "lowerit.h"
#include "getEnvConfig.h"
#include "sql_getpw.h"
#include "parse_quota.h"
#include "recalc_quota.h"
#include "sql_setquota.h"

#ifndef	lint
static char     sccsid[] = "$Id: setuserquota.c,v 1.1 2019-04-14 18:28:01+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_gethostid: out of memory", 0);
	_exit(111);
}

/*- Update a users quota */
int
setuserquota(char *username, char *domain, char *quota)
{
	char           *domain_ptr;
	char            strnum[FMT_ULONG];
	static stralloc tmp = {0};
	int             i;
	struct passwd  *pw;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit, mailcount;
#endif

	if (!username || !*username) {
		strerr_warn1("setuserquota: username cannot be null", 0);
		return (-1);
	}
	lowerit(username);
	if (domain && *domain)
		domain_ptr = domain;
	else
		getEnvConfigStr(&domain_ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	lowerit(domain_ptr);
	if ((size_limit = parse_quota(quota, &count_limit)) == -1) {
		strerr_warn3("setuserquota: parse_quota: ", quota, ": ", &strerr_sys);
		return (1);
	}
	if (count_limit) {
		if (!stralloc_copyb(&tmp, strnum, fmt_ulong(strnum, (unsigned long) size_limit)) ||
				!stralloc_catb(&tmp, "S,", 2) ||
				!stralloc_catb(&tmp, strnum, fmt_ulong(strnum, (unsigned long) count_limit)) ||
				!stralloc_catb(&tmp, "C", 1) ||
				!stralloc_0(&tmp))
			die_nomem();
	} else {
		if (!stralloc_copyb(&tmp, strnum, fmt_ulong(strnum, (unsigned long) size_limit)) ||
				!stralloc_0(&tmp))
			die_nomem();
	}
	if ((i = sql_setquota(username, domain_ptr, tmp.s)) == -1) {
		strerr_warn3("setuserquota: Failed to set quota to [", tmp.s, "]", 0);
		return (-1);
	} else
	if (!i)
		return (-1);
	if (!(pw = sql_getpw(username, domain_ptr))) {
		strerr_warn5("setuserquota: ", username, "@", domain_ptr, ": no such user ", 0);
		return (1);
	}
	if (!stralloc_copys(&tmp, pw->pw_dir) || !stralloc_catb(&tmp, "/Maildir", 8)
			|| !stralloc_0(&tmp))
		die_nomem();
#ifdef USE_MAILDIRQUOTA
	if ((size_limit = parse_quota(quota, &count_limit)) == -1) {
		strerr_warn3("setuserquota: parse_quota: ", quota, ": ", &strerr_sys);
		return (1);
	}
	return (recalc_quota(tmp.s, &mailcount, size_limit, count_limit, 2));
#else
	return (recalc_quota(tmp.s, 2))
#endif
}
