/*
 * $Log: parse_email.c,v $
 * Revision 1.3  2021-07-27 18:06:44+05:30  Cprogrammer
 * set default domain using vset_default_domain
 *
 * Revision 1.2  2020-04-01 18:57:26+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-14 18:35:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <getEnvConfig.h>
#endif
#include "vset_default_domain.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: parse_email.c,v 1.3 2021-07-27 18:06:44+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * parse out user and domain from an email address utility function
 * 
 * email  = input email address
 * user   = parsed user
 * domain = parsed domain
 * return   0 success
 *         -1 if either user or domain was truncated due to buff_size being reached
 */
int
parse_email(char *email, stralloc *user, stralloc *domain)
{
	char           *ptr;
	int             i, len;

	for (len = 0, ptr = email; *ptr; ptr++, len++) {
		i = str_chr(ATCHARS, *ptr);
		if (ATCHARS[i])
			break;
	}
	if (len) {
		if (!stralloc_copyb(user, email, len) || !stralloc_0(user))
			return (-1);
		user->len--;
	} else {
		if (!stralloc_0(user))
			return (-1);
		user->len = 0;
	}
	if (*ptr)
		ptr++;
	else
		ptr = vset_default_domain();
	if (!stralloc_copys(domain, ptr) || !stralloc_0(domain))
		return (-1);
	domain->len--;
	return (0);
}
