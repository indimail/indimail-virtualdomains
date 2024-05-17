/*
 * $Log: RelayInLookup.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-18 07:56:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: RelayInLookup.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

/*
 *  0: User not authenticated (no entry in relay table)
 *  1: User Authenticated.
 * -1: System Error
 */
#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "sqlOpen_user.h"
#else
#include "iopen.h"
#endif
#include "variables.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "relay_select.h"

static void
die_nomem()
{
	strerr_warn1("RelayInLookup: out of memory", 0);
	_exit(111);
}

int
RelayInLookup(const char *mailfrom, const char *remoteip)
{
	static stralloc user = {0}, domain = {0}, email = {0};
	const char     *real_domain;
	int             retval;

#ifdef CLUSTERED_SITE
	if (sqlOpen_user(mailfrom, 1))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound)
			return (0);
		else
			return (-1);
	}
	parse_email(mailfrom, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	if (!stralloc_copy(&email, &user) ||
			!stralloc_append(&email, "@") ||
			!stralloc_cats(&email, real_domain) ||
			!stralloc_0(&email))
		die_nomem();
	retval = relay_select(email.s, remoteip);
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return (retval);
}
#else
int
RelayInLookup(char *mailfrom, char *remoteip)
{
	return (-2);
}
#endif
