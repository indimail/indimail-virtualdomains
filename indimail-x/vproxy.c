/*
 * $Log: vproxy.c,v $
 * Revision 1.1  2019-04-18 08:38:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#endif
#include "monkey.h"

#ifndef	lint
static char     sccsid[] = "$Id: vproxy.c,v 1.1 2019-04-18 08:38:45+05:30 Cprogrammer Exp mbhangui $";
#endif

int
main(int argc, char **argv)
{
	if(argc == 3)
		return (monkey(argv[1], argv[2], 0, 0));
	else
	if(argc == 4)
		return (monkey(argv[1], argv[2], argv[3], 0));
	strerr_warn1("usage: vproxy host port [login sequence]", 0);
	return (1);
}
