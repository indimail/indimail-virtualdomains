/*
 * $Log: pwcomp.c,v $
 * Revision 1.2  2022-10-20 11:58:11+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.1  2019-04-14 21:05:08+05:30  Cprogrammer
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
#include <str.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: pwcomp.c,v 1.2 2022-10-20 11:58:11+05:30 Cprogrammer Exp mbhangui $";
#endif

int
pwcomp(struct passwd *pw1, struct passwd *pw2)
{
	if (!pw1 || !pw2)
		return (1);
	else
	if (str_diff(pw1->pw_name, pw2->pw_name))
		return (1);
	else
	if (str_diff(pw1->pw_passwd, pw2->pw_passwd))
		return (1);
	else
	if (pw1->pw_uid != pw2->pw_uid)
		return (1);
	else
	if (pw1->pw_gid != pw2->pw_gid)
		return (1);
	else
	if (str_diff(pw1->pw_gecos, pw2->pw_gecos))
		return (1);
	else
	if (str_diff(pw1->pw_dir, pw2->pw_dir))
		return (1);
	else
	if (str_diff(pw1->pw_shell, pw2->pw_shell))
		return (1);
	return (0);
}
