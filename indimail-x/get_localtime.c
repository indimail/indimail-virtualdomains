/*
 * $Log: get_localtime.c,v $
 * Revision 1.2  2020-09-17 14:47:36+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.1  2019-04-20 09:18:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: get_localtime.c,v 1.2 2020-09-17 14:47:36+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("get_localtime: out of memory", 0);
	_exit(111);
}

char           *
get_localtime()
{
	time_t          tmval;
	struct tm      *tmptr;
	static stralloc tmpbuf = {0};
#ifdef USE_TIMEZONE
	int             mins, hours, len;
#endif

	tmval = time(0);
	tmptr = localtime(&tmval);
	if (!stralloc_copys(&tmpbuf, asctime(tmptr)))
		die_nomem();
	if (tmpbuf.s[tmpbuf.len - 1] == '\n')
		tmpbuf.len--; /*- remove newline */
#ifdef USE_TIMEZONE
	mins = (timezone % 3600) / 60;
	if (mins < 0)
		mins = -mins;
	hours = timezone / 3600;
	if (!stralloc_cats(&tmpbuf, tzname[0]) ||
			!stralloc_append(&tmpbuf, " ") ||
			!stralloc_catb(&tmpbuf, strnum, fmt_uint(strnum, hours)) ||
			!stralloc_append(&tmpbuf, ":"))
		die_nomem();
	if (mins < 10 && !stralloc_append(&tmpbuf, "0"))
		die_nomem();
	if (!stralloc_catb(&tmpbuf, strnum, fmt_uint(strnum, mins)))
		die_nomem();
#endif
	if (!stralloc_0(&tmpbuf))
		die_nomem();
	return (tmpbuf.s);
}
