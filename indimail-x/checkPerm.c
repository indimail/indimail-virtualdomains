/*
 * $Log: checkPerm.c,v $
 * Revision 1.1  2019-04-18 08:37:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: checkPerm.c,v 1.1 2019-04-18 08:37:42+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_QMAIL
#include <str.h>
#endif
#include "mgmtpassfuncs.h"
#include "vpriv.h"
#include "variables.h"

int
checkPerm(char *user, char  *program, char **argv)
{
	char           *ptr1, *ptr2, *tuser;
	char          **Ptr;

	if (!user || !*user || !program || !*program || !argv || !*argv)
		return (1);
	if (mgmtpassinfo(user, 0) && userNotFound)
		return (1);
	tuser = user;
	ptr2 = program;
	for(;;) {
		if (!(ptr1 = vpriv_select(&tuser, &ptr2)))
			break;
		if (!str_diffn(ptr1, "*", 2))
			return(0);
		for (Ptr = argv;Ptr && *Ptr;Ptr++) {
			if (!str_str(ptr1, *Ptr))
				return (1);
		}
		return (0);
	}
	return (1);
}
#endif
