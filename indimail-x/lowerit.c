/*
 * $Log: lowerit.c,v $
 * Revision 1.2  2022-10-20 11:57:56+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.1  2019-04-18 08:27:54+05:30  Cprogrammer
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
static char     sccsid[] = "$Id: lowerit.c,v 1.2 2022-10-20 11:57:56+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * make all characters in a string be lower case
 */
void
lowerit(char *instr)
{
	if (!instr || !*instr)
		return;
	for (; *instr != 0; ++instr)
		if (isupper((int) *instr))
			*instr = tolower(*instr);
}
