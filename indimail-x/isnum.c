/*
 * $Log: isnum.c,v $
 * Revision 1.1  2019-04-18 08:21:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: isnum.c,v 1.1 2019-04-18 08:21:44+05:30 Cprogrammer Exp mbhangui $";
#endif

int
isnum(char *str)
{
	register char  *ptr;

	for (ptr = str; *ptr; ptr++)
		if (!isdigit((int) *ptr))
			return (0);
	return (1);
}
