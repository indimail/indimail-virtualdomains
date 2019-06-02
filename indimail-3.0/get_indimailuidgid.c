/*
 * $Log: get_indimailuidgid.c,v $
 * Revision 1.1  2019-04-12 20:42:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: get_indimailuidgid.c,v 1.1 2019-04-12 20:42:49+05:30 Cprogrammer Exp mbhangui $";
#endif

int
get_indimailuidgid(uid_t *uid, gid_t *gid)
{
	struct passwd  *pw;
	static uid_t    suid = -1;
	static gid_t    sgid = -1;

	if (suid != -1 && sgid != -1) {
		*uid = suid;
		*gid = sgid;
		return (0);
	}
	if (!(pw = getpwnam(INDIMAILUSER)))
		strerr_die3sys(111, "getpwnam: ", INDIMAILUSER, ": ");
	*uid = suid = pw->pw_uid;
	*gid = sgid = pw->pw_gid;
	return (0);
}
