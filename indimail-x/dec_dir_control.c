/*
 * $Log: dec_dir_control.c,v $
 * Revision 1.1  2019-04-18 08:37:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include "dir_control.h"
#include "GetPrefix.h"
#include "backfill.h"

#ifndef	lint
static char     sccsid[] = "$Id: dec_dir_control.c,v 1.1 2019-04-18 08:37:44+05:30 Cprogrammer Exp mbhangui $";
#endif

int
dec_dir_control(const char *path, const char *user, const char *domain, uid_t uid, gid_t gid)
{
	const char     *ptr;

	if (!(ptr = GetPrefix(user, path)))
		return (-1);
	backfill(user, domain, path, 2);
	vread_dir_control(ptr, &vdir, domain);
	if (vdir.cur_users)
		vdir.cur_users--;
	return (vwrite_dir_control(ptr, &vdir, domain, uid, gid));
}
