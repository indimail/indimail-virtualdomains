/*
 * $Log: GetPrefix.c,v $
 * Revision 1.2  2020-04-01 18:54:56+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:25:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <getEnvConfig.h>
#endif
#include "pathToFilesystem.h"

#ifndef	lint
static char     sccsid[] = "$Id: GetPrefix.c,v 1.2 2020-04-01 18:54:56+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("GetPrefix: out of memory", 0);
	_exit(111);
}

char           *
GetPrefix(char *user, char *path)
{
	char           *ptr, *suffix_ptr, *base_path;
	int             ch;
	static stralloc PathPrefix = {0};

	if (!user || !*user)
		return (" ");
	if (path && *path)
		base_path = path;
	else
		getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	ch = tolower(*user);
	if (ch >= 'a' && ch <= 'e')
		suffix_ptr = "A2E";
	else
	if (ch >= 'f' && ch <= 'k')
		suffix_ptr = "F2K";
	else
	if (ch >= 'l' && ch <= 'p')
		suffix_ptr = "L2P";
	else
	if (ch >= 'q' && ch <= 's')
		suffix_ptr = "Q2S";
	else
		suffix_ptr = "T2Z";
	if (!(ptr = pathToFilesystem(base_path)))
		return ((char *) 0);
	if (!str_diffn(ptr, "/", 2))
		ptr = "root";
	if (!stralloc_cats(&PathPrefix, ptr) ||
			!stralloc_append(&PathPrefix, "_") ||
			!stralloc_catb(&PathPrefix, suffix_ptr, 3) ||
			!stralloc_0(&PathPrefix))
		die_nomem();
	for (ptr = PathPrefix.s; *ptr; ptr++)
		if (*ptr == '/')
			*ptr = '_';
	return (PathPrefix.s);
}
