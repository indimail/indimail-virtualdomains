/*
 * $Log: mkpasswd3.c,v $
 * Revision 1.1  2019-04-14 21:10:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_QMAIL
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#endif
#include "variables.h"
#include "makesalt.h"
#include "in_crypt.h"

#ifndef	lint
static char     sccsid[] = "$Id: mkpasswd3.c,v 1.1 2019-04-14 21:10:40+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("mkpasswd3: out of memory", 0);
	_exit(111);
}

/* encrypt a password */
int
mkpasswd3(char *newpasswd, stralloc *crypted)
{
	char           *tmpstr;
	char            salt[SALTSIZE + 1];
	static int      seeded;

	crypted->len = 0;
	if (encrypt_flag)
		tmpstr = newpasswd;
	else {
		if (!seeded) {
			seeded = 1;
			srandom(time(0)^(getpid()<<15));
		}
		makesalt(salt, SALTSIZE);
		if (!(tmpstr = in_crypt(newpasswd, salt)))
			return (-1);
	}
	if (!stralloc_copys(crypted, tmpstr) || !stralloc_0(crypted))
		die_nomem();
	crypted->len--;
	return (0);
}
