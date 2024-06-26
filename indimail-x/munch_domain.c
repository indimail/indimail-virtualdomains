/*
 * $Log: munch_domain.c,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-18 08:16:19+05:30  Cprogrammer
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
#endif
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: munch_domain.c,v 1.2 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("munch_domain: out of memory", 0);
	_exit(111);
}

const char     *
munch_domain(const char *domain)
{
	int             i;
	static stralloc tmpbuf = {0};

	if (!domain || !*domain)
		return (domain);
	if (!stralloc_copys(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
		die_nomem();
	for (i = 0; i < tmpbuf.len; i++) {
		if (tmpbuf.s[i] == '.' || tmpbuf.s[i] == '-')
			tmpbuf.s[i] = MYSQL_DOT_CHAR;
	}
	return (tmpbuf.s);
}
