/*
 * $Log: vgetpasswd.c,v $
 * Revision 1.1  2019-04-18 07:59:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <subfd.h>
#include <strerr.h>
#include <str.h>
#include <stralloc.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vgetpasswd.c,v 1.1 2019-04-18 07:59:28+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef sun
char           *getpass(char *);
#endif

static void
die_nomem()
{
	strerr_warn1("vgetpasswd: out of memory", 0);
	_exit(111);
}

/*
 * prompt the command line and get a password twice, that matches 
 */
char           *
vgetpasswd(user)
	char           *user;
{
	char           *ptr;
	static stralloc pass1 = {0}, pass2 = {0}, tmpstr = {0};

	if (!stralloc_copyb(&tmpstr, "New IndiMail password for ", 26) ||
			!stralloc_cats(&tmpstr, user) ||
			!stralloc_catb(&tmpstr, ": ", 2) ||
			!stralloc_0(&tmpstr))
		die_nomem();
	while (1) {
		if (!(ptr = getpass(tmpstr.s)))
			return ((char *) 0);
		if (!stralloc_copys(&pass1, ptr) || !stralloc_0(&pass1))
			die_nomem();
		if (!(ptr = getpass("Retype new IndiMail password: ")))
			return ((char *) 0);
		if (!stralloc_copys(&pass2, ptr) || !stralloc_0(&pass2))
			die_nomem();
		if (str_diff(pass1.s, pass2.s)) {
			if (substdio_puts(subfderr, "Passwords do not match, try again\n") == -1)
				strerr_die1sys(111, "vgetpasswd: write: ");
		} else
			break;
	}
	return (pass1.s);
}
