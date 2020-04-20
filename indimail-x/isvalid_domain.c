/*
 * $Log: isvalid_domain.c,v $
 * Revision 1.1  2019-04-18 08:25:37+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: isvalid_domain.c,v 1.1 2019-04-18 08:25:37+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <ctype.h>
#include "indimail.h"

int
isvalid_domain(char *domain)
{
	char           *ptr;
	register int    i;

	for (i = 0, ptr = domain; ptr && *ptr; ptr++, i++)
	{
		if (*ptr != '-' && *ptr != '.' && !isalnum((int) *ptr))
			return (0);
	}
	if (i > MAX_PW_DOMAIN)
		return (0);
	return (1);
}
