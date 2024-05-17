/*
 * $Log: is_distributed_domain.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-14 22:53:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <env.h>
#include <stralloc.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#endif
#include "LoadDbInfo.h"
#include "get_assign.h"

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

#ifndef	lint
static char     sccsid[] = "$Id: is_distributed_domain.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("is_distributed_domain: out of memory", 0);
	_exit(111);
}

int
is_distributed_domain(const char *Domain)
{
	static stralloc savedomain = {0};
	static int      dist_flag;
	int             total;
	DBINFO        **rhostsptr;

	if (!Domain || !*Domain)
		return (0);
#ifdef QUERY_CACHE
	if (_cacheSwitch && (char *) env_get("QUERY_CACHE") && dist_flag != -1 && savedomain.len
		&& !str_diffn(Domain, savedomain.s, savedomain.len + 1))
		return (dist_flag);
	else
	if (!stralloc_copys(&savedomain, Domain) || !stralloc_0(&savedomain))
		die_nomem();
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#else
	if (!stralloc_copys(&savedomain, Domain) || !stralloc_0(&savedomain))
		die_nomem();
#endif
	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total))) {
		if (errno == error_noent)
			return (dist_flag = 0);
		strerr_die1sys(111, "is_distributed_domain: LoadDbInfo_TXT: ");
	}
	for (dist_flag = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++) {
		if (!str_diffn((*rhostsptr)->domain, Domain, DBINFO_BUFF))
			return ((dist_flag = (*rhostsptr)->distributed));
	}
	return (dist_flag = 0);
}

#ifdef QUERY_CACHE
void
is_distributed_domain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#else
int
is_distributed_domain(char *Domain)
{
	return (0);
}
#endif
