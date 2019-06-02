/*
 * $Log: PwdInLookup.c,v $
 * Revision 1.1  2019-04-18 07:56:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif
#include "parse_email.h"
#ifdef CLUSTERED_SITE
#include "sqlOpen_user.h"
#else
#include "iopen.h"
#endif
#include "get_real_domain.h"
#include "sql_getpw.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: PwdInLookup.c,v 1.1 2019-04-18 07:56:24+05:30 Cprogrammer Exp mbhangui $";
#endif

struct passwd *
PwdInLookup(char *email)
{
	static stralloc user = {0}, domain = {0};
	char           *real_domain;
	struct passwd  *pw;

#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 1))
#else
	if (iopen((char *) 0))
#endif
		return ((struct passwd *) 0);
	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	pw = sql_getpw(user.s, real_domain);
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return (pw);
}
