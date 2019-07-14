/*
 * $Log: check_group.c,v $
 * Revision 1.2  2019-04-22 22:24:44+05:30  Cprogrammer
 * added condition including of qmail header files
 *
 * Revision 1.1  2019-04-12 20:15:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#endif
#include <unistd.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <alloc.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: check_group.c,v 1.2 2019-04-22 22:24:44+05:30 Cprogrammer Exp mbhangui $";
#endif

int
check_group(gid_t gid, char *str)
{
	int             size, i;
	gid_t          *list;

	if ((size = getgroups(0, (gid_t *) 0)) == -1) {
		if (str)
			strerr_die2sys(111, str, "check_group: getgroups: size: ");
		else
			strerr_die1sys(111, "check_group: getgroups: size: ");
	}
	if (!(list = (gid_t *) alloc(size * sizeof (gid_t)))) {
		if (str)
			strerr_die2sys(111, str, "check_group: alloc: ");
		else
			strerr_die1sys(111, "check_group: alloc: ");
	}
	if ((size = getgroups(size, list)) == -1) {
		if (str)
			strerr_die2sys(111, str, "check_group: getgroups: ");
		else
			strerr_die1sys(111, "check_group: getgroups: ");
	}
	for (i = 0; i < size; i++) {
		if (list[i] == gid)
			return (1);
	}
	return (0);
}
