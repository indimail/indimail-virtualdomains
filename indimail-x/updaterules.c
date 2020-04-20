/*
 * $Log: updaterules.c,v $
 * Revision 1.2  2019-04-22 23:16:01+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 08:37:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#endif
#include "iopen.h"
#include "iclose.h"
#include "update_rules.h"

#ifndef	lint
static char     sccsid[] = "$Id: updaterules.c,v 1.2 2019-04-22 23:16:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
int
main()
{
	if (iopen((char *) 0))
		return(1);
	if (update_rules(1)) {
		iclose();
		return(1);
	}
	iclose();
	return(0);
}
#else
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-roaming-users=y", 0);
	return(1);
}
#endif
