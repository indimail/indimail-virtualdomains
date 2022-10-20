/*
 * $Log: passwd_policy.c,v $
 * Revision 1.3  2022-10-20 11:58:08+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.2  2021-07-22 15:17:23+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.1  2019-04-14 21:10:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <strerr.h>
#endif

#ifndef lint
static char     sccsid[] = "$Id: passwd_policy.c,v 1.3 2022-10-20 11:58:08+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      get_dict(char *);
static int      get_hist(char *);

int
passwd_policy(char *passwd)
{
	char           *ptr;
	int             len, alpha, numeric;

	if ((len = str_len(passwd)) < 8) {
		strerr_warn1("passwd must be of minimum 8 chars", 0);
		return (1);
	}
	for (ptr = passwd, alpha = numeric = 0; *ptr; ptr++) {
		if (isspace((int) *ptr)) {
			strerr_warn1("whitespace not allowed", 0);
			return (1);
		}
		if (*ptr && *(ptr + 1) && (*ptr == *(ptr + 1))) {
			strerr_warn1("two consequtive chars cannot be same", 0);
			return (1);
		}
		if (isdigit((int) *ptr))
			numeric = 1;
		if (isalpha((int) *ptr))
			alpha = 1;
	}
	if (!alpha || !numeric) {
		strerr_warn1("passwd must be alpha-numeric", 0);
		return (1);
	}
	if (get_dict(passwd)) {
		strerr_warn1("passwd is restricted", 0);
		return (1);
	}
	if (get_hist(passwd)) {
		strerr_warn1("passwd cannot be reused", 0);
		return (1);
	}
	return (0);
}

static int
get_dict(char *passwd)
{
	return (0);
}

static int
get_hist(char *passwd)
{
	return (0);
}
