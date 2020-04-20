/*
 * $Log: copyPwdStruct.c,v $
 * Revision 1.1  2019-04-14 20:12:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: copyPwdStruct.c,v 1.1 2019-04-14 20:12:08+05:30 Cprogrammer Exp mbhangui $";
#endif

struct passwd  *
copyPwdStruct(struct passwd *pw)
{
	static stralloc IUser = {0}, IPass = {0}, IGecos = {0},
					IDir = {0}, IShell = {0};
	static struct passwd pwent;

	if (!stralloc_copys(&IUser, pw->pw_name) || !stralloc_0(&IUser))
		return ((struct passwd *) 0);
	if (!stralloc_copys(&IPass, pw->pw_passwd) || !stralloc_0(&IPass))
		return ((struct passwd *) 0);
	if (!stralloc_copys(&IGecos, pw->pw_gecos) || !stralloc_0(&IGecos))
		return ((struct passwd *) 0);
	if (!stralloc_copys(&IDir, pw->pw_dir) || !stralloc_0(&IDir))
		return ((struct passwd *) 0);
	if (!stralloc_copys(&IShell, pw->pw_shell) || !stralloc_0(&IShell))
		return ((struct passwd *) 0);
	pwent.pw_name = IUser.s;
	pwent.pw_passwd = IPass.s;
	pwent.pw_uid = pw->pw_uid;
	pwent.pw_gid = pw->pw_gid;
	pwent.pw_gecos = IGecos.s;
	pwent.pw_dir = IDir.s;
	pwent.pw_shell = IShell.s;
	return (&pwent);
}
