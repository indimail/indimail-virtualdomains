/*
 * $Log: VlimitInLookup.c,v $
 * Revision 1.1  2019-04-18 07:56:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: VlimitInLookup.c,v 1.1 2019-04-18 07:56:47+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_DOMAIN_LIMITS
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
#include "get_real_domain.h"
#include "variables.h"
#include "vlimits.h"

int
VlimitInLookup(char *email, struct vlimits *lim)
{
	static stralloc user = {0}, domain = {0};
	char           *real_domain;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 1))
#else
	if (iopen((char *) 0))
#endif
	{
		if(userNotFound)
			return(1);
		else
			return(-1);
	}
	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	if (vget_limits(real_domain, &limits)) {
		strerr_warn3("VlimitInLookup: ", real_domain, ": failed to get domain limits", 0);
		return (-1);
	}
	*lim = limits;
	return (0);
}
#endif
