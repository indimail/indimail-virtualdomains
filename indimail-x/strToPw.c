/*
 * $Log: strToPw.c,v $
 * Revision 1.2  2019-04-16 15:14:19+05:30  Cprogrammer
 * fix for getting all fields
 *
 * Revision 1.1  2019-04-14 20:55:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: strToPw.c,v 1.2 2019-04-16 15:14:19+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("strToPw: out of memory", 0);
	_exit(111);
}

struct passwd  *
strToPw(char *pwbuf, int len)
{
	char           *ptr, *cptr, *tmp;
	int             row_count, pwstruct_len;
	static struct passwd pwent;
	static stralloc __PWstruct = {0}, _pwstruct = {0};
	static stralloc IUser = {0}, IPass = {0}, IGecos = {0}, IDir = {0}, IShell = {0};

	if (!pwbuf || !*pwbuf)
		return ((struct passwd *) 0);
	/* len includes one null character */
	if (!str_diffn(pwbuf, "PWSTRUCT=", 9)) {
		pwstruct_len = len - 9;
		if (!stralloc_copyb(&_pwstruct, pwbuf + 9, pwstruct_len - 1))
			die_nomem();
	} else {
		pwstruct_len = len;
		if (!stralloc_copyb(&_pwstruct, pwbuf, pwstruct_len - 1))
			die_nomem();
	}
	if (!stralloc_append(&_pwstruct, ":") ||
			!stralloc_0(&_pwstruct))
		die_nomem();
	_pwstruct.len -= 2;
	if (__PWstruct.len && !str_diffn(__PWstruct.s, _pwstruct.s, pwstruct_len)) {
		if (IUser.len && IPass.len && IGecos.len && IDir.len && IShell.len)
			return ((struct passwd *) &pwent);
		else
			return ((struct passwd *) 0);
	}
	is_overquota = is_inactive = userNotFound = 0;
	if (!stralloc_copy(&__PWstruct, &_pwstruct) || !stralloc_0(&__PWstruct))
		die_nomem();
	__PWstruct.len--;
	if (!str_diffn(_pwstruct.s, "No such user", 12)) {
		userNotFound = 1;
		return ((struct passwd *) 0);
	}
	for (row_count = 0, cptr = ptr = _pwstruct.s; *ptr; ptr++) {
		if (*ptr == ':')
			*ptr = 0;
		else
			continue;
		switch (row_count)
		{
		case 0: /*- user */
			for (tmp = cptr;*tmp; tmp++) {
				if (*tmp == '@') {
					*tmp = 0;
					break;
				}
			}
			if (!stralloc_copys(&IUser, cptr) || !stralloc_0(&IUser))
				die_nomem();
			IUser.len--;
			pwent.pw_name = IUser.s;
			break;
		case 1:
			if (!stralloc_copys(&IPass, cptr) || !stralloc_0(&IPass))
				die_nomem();
			IPass.len--;
			pwent.pw_passwd = IPass.s;
			break;
		case 2:
			scan_uint(cptr, &pwent.pw_uid);
			break;
		case 3:
			scan_uint(cptr, &pwent.pw_gid);
			if (pwent.pw_gid & BOUNCE_MAIL)
				is_overquota = 1;
			break;
		case 4:
			if (!stralloc_copys(&IGecos, cptr) || !stralloc_0(&IGecos))
				die_nomem();
			IGecos.len--;
			pwent.pw_gecos = IGecos.s;
			break;
		case 5:
			if (!stralloc_copys(&IDir, cptr) || !stralloc_0(&IDir))
				die_nomem();
			IDir.len--;
			pwent.pw_dir = IDir.s;
			break;
		case 6:
			if (!stralloc_copys(&IShell, cptr) || !stralloc_0(&IShell))
				die_nomem();
			IShell.len--;
			pwent.pw_shell = IShell.s;
			break;
		case 7:
			scan_int(cptr, &is_inactive);
			break;
		}
		cptr = ptr + 1;
		row_count++;
		if (row_count == 8)
			return (&pwent);
	} /*- for (row_count = 0, cptr = ptr = _pwstruct.s; *ptr; ptr++) */
	return ((struct passwd *) 0);
}
