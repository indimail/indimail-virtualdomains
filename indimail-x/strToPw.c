/*
 * $Id: strToPw.c,v 1.6 2023-07-15 00:22:57+05:30 Cprogrammer Exp mbhangui $
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
static char     sccsid[] = "$Id: strToPw.c,v 1.6 2023-07-15 00:22:57+05:30 Cprogrammer Exp mbhangui $";
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
	int             row_count, colon_count, pwstruct_len, is_scram = 0;
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
	/*
	 * This will append a ':', result of which
	 * will be pwstruct will have 8 colons
	 */
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
	for (colon_count = 0, ptr = _pwstruct.s; *ptr; ptr++)
		if (*ptr == ':')
			colon_count++;
	for (row_count = 0, cptr = ptr = _pwstruct.s; *ptr; ptr++) {
		if (*ptr == ':') {
			/*-
			 * The scram string may have two ':', causing a problem
			 * of extracting password field tokens based on ':' as
			 * the separator.
			 * so we skip past hexsaltedpw and saltedpw
			 * to get the uid in the next iteration instead of hexsaltedpw
			 */
			if (colon_count > 8 && row_count == 1 && !str_diffn(cptr, "{SCRAM-SHA-", 11)) {
				for (ptr += 12; *ptr; ptr++) {
					if (*ptr == ':') {
						is_scram++;
						if (is_scram == 2)
							break;
					}
				}
			}
			*ptr = 0;
		} else
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
		case 1: /*- password */
			if (!stralloc_copys(&IPass, cptr) || !stralloc_0(&IPass))
				die_nomem();
			IPass.len--;
			pwent.pw_passwd = IPass.s;
			break;
		case 2: /*- uid */
			scan_uint(cptr, &pwent.pw_uid);
			break;
		case 3: /*- gid */
			scan_uint(cptr, &pwent.pw_gid);
			if (pwent.pw_gid & BOUNCE_MAIL)
				is_overquota = 1;
			break;
		case 4: /*- gecos */
			if (!stralloc_copys(&IGecos, cptr) || !stralloc_0(&IGecos))
				die_nomem();
			IGecos.len--;
			pwent.pw_gecos = IGecos.s;
			break;
		case 5: /*- home dir */
			if (!stralloc_copys(&IDir, cptr) || !stralloc_0(&IDir))
				die_nomem();
			IDir.len--;
			pwent.pw_dir = IDir.s;
			break;
		case 6: /*- quota */
			if (!stralloc_copys(&IShell, cptr) || !stralloc_0(&IShell))
				die_nomem();
			IShell.len--;
			pwent.pw_shell = IShell.s;
			break;
		case 7: /*- active/inactive flag */
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

/*
 * $Log: strToPw.c,v $
 * Revision 1.6  2023-07-15 00:22:57+05:30  Cprogrammer
 * updated comments
 *
 * Revision 1.5  2022-08-25 20:50:25+05:30  Cprogrammer
 * use colon_count to fix logic for cram/non-cram passwords
 *
 * Revision 1.4  2022-08-25 18:11:59+05:30  Cprogrammer
 * handle additional hex salted passwod and clear text password in pw_passwd field
 *
 * Revision 1.3  2022-08-04 14:42:08+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.2  2019-04-16 15:14:19+05:30  Cprogrammer
 * fix for getting all fields
 *
 * Revision 1.1  2019-04-14 20:55:14+05:30  Cprogrammer
 * Initial revision
 *
 */
