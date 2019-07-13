/*
 * $Log: setuserid.c,v $
 * Revision 1.1  2019-04-14 21:02:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <str.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: setuserid.c,v 1.1 2019-04-14 21:02:09+05:30 Cprogrammer Exp mbhangui $";
#endif
/*-
 * scan the group file for all supplementary groups.
 * Return NULL if unsucessfull
 * The returned value should be freed after use
 */
gid_t   *
grpscan(char *user, int *ngroups)
{
	struct passwd  *pwd;
	struct group   *grp;
	long            maxgroups, idx;
	gid_t          *gidsetlen;
	char          **ptr;

	if (!user || !*user)
		return ((gid_t *) 0);
	if ((maxgroups = sysconf(_SC_NGROUPS_MAX)) == -1)
		return ((gid_t *) 0);
	else
	if (!(gidsetlen = (gid_t *) alloc(maxgroups * sizeof(gid_t))))
		return ((gid_t *) 0);
	else
	if (!(pwd = getpwnam(user)))
		return ((gid_t *) 0);
	idx = 0;
	gidsetlen[idx++] = pwd->pw_gid;	/* the base gid */
	endpwent();
	for (;;) {
		if (!(grp = getgrent()))
			break;
		for (ptr = grp->gr_mem; *ptr; ptr++) {
			if (!str_diff(user, *ptr) && grp->gr_gid != gidsetlen[0])
				gidsetlen[idx++] = grp->gr_gid;	/* supplementary group ids */
		}
	}
	endgrent();
	*ngroups = idx;
	return (gidsetlen);
}


int
setuserid(char *user)
{
	struct passwd  *pwdent;
	gid_t          *gidset;
	int             ngroups;
	uid_t           uid;
	gid_t           gid;

	if (!(pwdent = getpwnam(user)))
		return (-1);
	uid = pwdent->pw_uid;
	gid = pwdent->pw_gid;
	endpwent();
	if (!(gidset = grpscan(user, &ngroups)))
		return (-1);
	if (setgroups(ngroups, gidset)) {
		alloc_free((char *) gidset);
		return (-1);
	} else
	if (setgid(gid)) {
		alloc_free((char *) gidset);
		return (-1);
	} else
	if (setuid(uid)) {
		alloc_free((char *) gidset);
		return (-1);
	}
	alloc_free((char *) gidset);
	return (0);
}

int
setuser_privileges(uid_t uid, gid_t gid, char *user)
{
	gid_t          *gidset;
	int             ngroups;

	if (!(gidset = grpscan(user, &ngroups)))
		return (-1);
	if (setgroups(ngroups, gidset)) {
		alloc_free((char *) gidset);
		return (-1);
	} else
	if (setgid(gid)) {
		alloc_free((char *) gidset);
		return (-1);
	} else
	if (setuid(uid)) {
		alloc_free((char *) gidset);
		return (-1);
	}
	alloc_free((char *) gidset);
	return (0);
}
