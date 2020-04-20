/*
 * $Log: remove_quotes.c,v $
 * Revision 1.1  2019-04-18 08:36:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: remove_quotes.c,v 1.1 2019-04-18 08:36:19+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("remove_quotes: out of memory", 0);
	_exit(111);
}

int
remove_quotes(char **address)
{
	char           *tmpstr, *ptr;
	static stralloc addr = {0};
	size_t          len;

	if (!address || !*address)
		return (1);
	for (ptr = *address; *ptr && isspace((int) *ptr); ptr++);
	if (!*ptr)
		return (1);
	if (*ptr == '\"') {
		ptr++;
		tmpstr = ptr;
		for (len = 0; *ptr && *ptr != '\"'; len++, ptr++);
		if (!stralloc_copyb(&addr, tmpstr, len) || !stralloc_0(&addr))
			die_nomem();
		*address = addr.s;
		return (0);
	}
	return (0);
}
