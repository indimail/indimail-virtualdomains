/*
 * $Log: get_localtime.c,v $
 * Revision 1.3  2022-10-22 12:53:49+05:30  Cprogrammer
 * use date822fmt() for local time
 *
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
/*
#ifdef HAVE_TIME_H
#include <time.h>
#endif
*/
#ifdef HAVE_QMAIL
#include <datetime.h>
#include <date822fmt.h>
#include <now.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: get_localtime.c,v 1.3 2022-10-22 12:53:49+05:30 Cprogrammer Exp mbhangui $";
#endif

char           *
get_localtime()
{
	struct datetime dt;
	int             len;
	static char     buf[DATE822FMT];

	datetime_tai(&dt, now());
	len = date822fmt(buf, &dt);
	buf[len - 1] = 0;
	return buf;
}
