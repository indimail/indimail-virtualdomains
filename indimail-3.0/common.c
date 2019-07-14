/*
 * $Log: common.c,v $
 * Revision 1.1  2019-04-14 20:57:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <substdio.h>
#include <strerr.h>
#include <subfd.h>

void
out(char *prefix, char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, prefix, ": write: ");
	return;
}

void
flush(char *prefix)
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, prefix, ": write: ");
}

void
errout(char *prefix, char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, prefix, ": write: ");
	return;
}

void
errflush(char *prefix)
{
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, prefix, ": write: ");
}
