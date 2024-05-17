/*
 * $Log: close_big_dir.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-18 08:25:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: close_big_dir.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

int
close_big_dir(const char *table_name, const char *domain, uid_t uid, gid_t gid)
{
	return (vwrite_dir_control(table_name, &vdir, domain, uid, gid));
}
