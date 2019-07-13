/*
 * $Log: Check_Login.c,v $
 * Revision 1.1  2019-04-18 08:31:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: Check_Login.c,v 1.1 2019-04-18 08:31:54+05:30 Cprogrammer Exp mbhangui $";
#endif

int
Check_Login(service, domain, type)
	const char *service, *domain, *type;
{
	static stralloc tmp1 = {0}, tmp2 = {0};

	if (!stralloc_copys(&tmp1, CONTROLDIR) ||
			!stralloc_append(&tmp1, "/") ||
			!stralloc_cats(&tmp1, (char *) domain) ||
			!stralloc_append(&tmp1, "/") ||
			!stralloc_cats(&tmp1, (char *) service) ||
			!stralloc_catb(&tmp1, "/nologin", 8) ||
			!stralloc_0(&tmp1))
		return (-1);
	if (!stralloc_copys(&tmp2, CONTROLDIR) ||
			!stralloc_append(&tmp2, "/") ||
			!stralloc_cats(&tmp2, (char *) type) ||
			!stralloc_append(&tmp2, "/") ||
			!stralloc_cats(&tmp2, (char *) service) ||
			!stralloc_catb(&tmp2, "/nologin", 8) ||
			!stralloc_0(&tmp2))
		return (-1);
	if(!access(tmp1.s, F_OK) || !access(tmp2.s, F_OK)) {
		out("Check_Login", "Login not permitted for ");
		out("Check_login", (char *) service);
		out("Check_login", "\n");
		flush("Check_Login");
		strerr_warn2("Login not permitted for ", (char *) service, 0);
		_exit(1);
	}
	return(0);
}
