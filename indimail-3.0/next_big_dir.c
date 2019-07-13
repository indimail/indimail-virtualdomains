/*
 * $Log: next_big_dir.c,v $
 * Revision 1.1  2019-04-18 08:31:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "dir_control.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: next_big_dir.c,v 1.1 2019-04-18 08:31:41+05:30 Cprogrammer Exp mbhangui $";
#endif

char           *
next_big_dir(uid_t uid, gid_t gid, int users_per_level)
{
	inc_dir_control(&vdir, users_per_level);

	return (vdir.the_dir);
}
