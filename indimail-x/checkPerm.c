/*
 * $Log: checkPerm.c,v $
 * Revision 1.2  2024-05-23 17:23:00+05:30  Cprogrammer
 * added feature to match command line args using regex/wildmatch
 *
 * Revision 1.1  2019-04-18 08:37:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: checkPerm.c,v 1.2 2024-05-23 17:23:00+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_QMAIL
#include <matchregex.h>
#include <wildmat.h>
#include <str.h>
#include <env.h>
#endif
#include "mgmtpassfuncs.h"
#include "vpriv.h"
#include "variables.h"

int
checkPerm(char *user, const char  *program, const char *cmdline)
{
	char           *ptr1, *ptr2, *tuser, *qregex;
	const char     *ptr;

	if (!user || !*user || !program || !*program || !cmdline || !*cmdline)
		return (1);
	if (mgmtpassinfo(user, 0) && userNotFound)
		return (1);
	tuser = user;
	ptr2 = (char *) program;
	qregex = env_get("QREGEX");
	for(;;) {
		if (!(ptr1 = vpriv_select(&tuser, &ptr2)))
			break;
		if (!str_diffn(ptr1, "*", 2))
			return (0);
		for (ptr = cmdline; *ptr; ptr++)
			if (isspace(*ptr))
				break;
		for (; *ptr && isspace(*ptr); ptr++);
		if (qregex ? matchregex(ptr, ptr1, 0) : wildmat(ptr, ptr1))
			return (0);
	}
	return (1);
}
#endif
