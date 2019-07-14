/*
 * $Log: Dirname.c,v $
 * Revision 1.1  2019-04-18 08:38:39+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: Dirname.c,v 1.1 2019-04-18 08:38:39+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("Dirname: out of memory", 0);
	_exit(111);
}

char           *
Dirname(char *path)
{
	static stralloc tmpbuf = {0};
	int             i;

	if (!path || !*path)
		return ((char *) 0);
	if (!stralloc_copys(&tmpbuf, path) || !stralloc_0(&tmpbuf))
		die_nomem();
	i = str_rchr(tmpbuf.s, '/');
	if (tmpbuf.s[i]) {
		if (!i)
			return ("/");
		tmpbuf.s[i] = 0;
		return (tmpbuf.s);
	} else
		return ((char *) 0);
}
