/*
 * $Id: common.c,v 1.7 2024-05-30 22:57:43+05:30 Cprogrammer Exp mbhangui $
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
#include <qprintf.h>
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "common.h"

void
my_exit(substdio *ss, int ret, int line, const char *fn)
{
	if (ss) {
		subprintf(ss, "%s: exit at line %d ret=%d\nEnd Time=%ld\n\n", fn, line, ret, time(0));
		substdio_flush(ss);
	}
	flush();
	_exit(ret);
}

void
die_nomem()
{
	extern char    *html_text[MAX_LANG_STR + 1];

	out(html_text[201]);
	out("1<BR>\n");
	flush();
	iclose();
	iweb_exit(MEMORY_FAILURE);
}

void
set_status_mesg_size(int len)
{
	if (!stralloc_ready(&StatusMessage, len))
		die_nomem();
}

void
copy_status_mesg(const char *str)
{
	if (!stralloc_copys(&StatusMessage, str) ||
			!stralloc_append(&StatusMessage, "\n") ||
			!stralloc_0(&StatusMessage))
		die_nomem();
	StatusMessage.len--;
}

void
out(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdoutsmall, str) == -1)
		iweb_exit(OUTPUT_FAILURE);
	return;
}

void
flush()
{
	if (substdio_flush(subfdoutsmall) == -1)
		iweb_exit(OUTPUT_FAILURE);
}

void
errout(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		iweb_exit(OUTPUT_FAILURE);
	return;
}

void
errflush()
{
	if (substdio_flush(subfderr) == -1)
		iweb_exit(OUTPUT_FAILURE);
}

/*
 * $Log: common.c,v $
 * Revision 1.7  2024-05-30 22:57:43+05:30  Cprogrammer
 * iweb_exit - flush output before exit
 *
 * Revision 1.6  2024-05-17 16:17:42+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-07-28 22:29:02+05:30  Cprogrammer
 * added my_exit() function to record source file, line no of exit
 *
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
