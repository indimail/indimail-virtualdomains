/*
 * $Log: AliasInLookup.c,v $
 * Revision 1.2  2019-07-04 09:15:20+05:30  Cprogrammer
 * BUG - aliasbuffer wasn't initialized
 *
 * Revision 1.1  2019-04-18 07:56:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: AliasInLookup.c,v 1.2 2019-07-04 09:15:20+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#ifdef CLUSTERED_SITE
#include "sqlOpen_user.h"
#else
#include "iopen.h"
#endif
#include "parse_email.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "valias_select.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("AliasInLookup: out of memory", 0);
	_exit(111);
}

char *
AliasInLookup(const char *email)
{
	static stralloc user = {0}, domain = {0}, aliasbuf = {0};
	char           *ptr;
	const char     *real_domain;

#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 1))
#else
	if (iopen((char *) 0))
#endif
		return((char *) 0);
	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	for (aliasbuf.len = 0;;) {
		if (!(ptr = valias_select(user.s, real_domain)))
			break;
		if (!stralloc_cats(&aliasbuf, ptr) || !stralloc_append(&aliasbuf, "\n"))
			die_nomem();
	}
	if (!stralloc_0(&aliasbuf))
		die_nomem();
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return(aliasbuf.s);
}
#else
char *
AliasInLookup(char *email)
{
	return((char *) 0);
}
#endif
