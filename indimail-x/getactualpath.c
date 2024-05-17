/*
 * $Log: getactualpath.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-18 08:35:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: getactualpath.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("getactualpath: out of memory", 0);
	_exit(111);
}

char    *
getactualpath(const char *path)
{
	char           *ptr;
	static stralloc pathbuf = {0};

	if (!(ptr = realpath(path, 0)))
		return ((char *) 0);
	if (!stralloc_copys(&pathbuf, ptr) || !stralloc_0(&pathbuf))
		die_nomem();
	free (ptr);
	return (pathbuf.s);
}
