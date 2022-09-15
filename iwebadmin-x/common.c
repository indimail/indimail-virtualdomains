/*
 * $Log: common.c,v $
 * Revision 1.4  2022-09-15 23:11:09+05:30  Cprogrammer
 * display out of memory error message in die_nomem()
 *
 * Revision 1.3  2021-03-14 12:47:42+05:30  Cprogrammer
 * prevent including mysql.h in indimail.h
 *
 * Revision 1.2  2020-10-30 12:43:40+05:30  Cprogrammer
 * add newline to separate functions
 *
 * Revision 1.1  2019-06-08 18:15:55+05:30  Cprogrammer
 * Initial revision
 *
 * Revision 1.1  2019-04-14 20:57:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define REMOVE_MYSQL_H
#include <indimail_compat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <substdio.h>
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include "iwebadmin.h"
#include "iwebadminx.h"

void
set_status_mesg_size(int len)
{
	if (!stralloc_ready(&StatusMessage, len))
		strerr_die1sys(111, "iwebadmin: out of memory: ");
}

void
copy_status_mesg(char *str)
{
	if (!stralloc_copys(&StatusMessage, str) ||
			!stralloc_append(&StatusMessage, "\n") ||
			!stralloc_0(&StatusMessage))
		strerr_die1sys(111, "iwebadmin: out of memory: ");
	StatusMessage.len--;
}

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdoutsmall, str) == -1)
		strerr_die1sys(111, "iwebadmin: write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdoutsmall) == -1)
		strerr_die1sys(111, "iwebadmin: write: ");
}

void
die_nomem()
{
	extern char    *html_text[MAX_LANG_STR + 1];

	out(html_text[201]);
	out("1<BR>\n");
	flush();
	iclose();
	_exit(0);
}

void
errout(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die1sys(111, "iwebadmin: write: ");
	return;
}

void
errflush()
{
	if (substdio_flush(subfderr) == -1)
		strerr_die1sys(111, "iwebadmin: write: ");
}
