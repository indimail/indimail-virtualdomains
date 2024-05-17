/*
 * $Log: open_big_dir.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-18 08:31:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "GetPrefix.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_big_dir.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

char *
open_big_dir(const char *username, const char *domain, char *path)
{
	char           *filesys_prefix;

	if (!(filesys_prefix = GetPrefix(username, path)))
		return ((char *) 0);
	if (vread_dir_control(filesys_prefix, &vdir, domain))
		return ((char *) 0);
	return (filesys_prefix);
}
