/*
 * $Log: common.c,v $
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <substdio.h>
#include <stralloc.h>
#include <strerr.h>
#include <qprintf.h>
#include <subfd.h>
#include "common.h"

void
out(const char *prefix, const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, prefix, ": write: ");
	return;
}

void
flush(const char *prefix)
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, prefix, ": write: ");
}

void
errout(const char *prefix, const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, prefix, ": write: ");
	return;
}

void
errflush(const char *prefix)
{
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, prefix, ": write: ");
}

#ifdef HAVE_STDARG_H
int             vsnprintf(char *str, size_t size, const char *format, va_list arg);
#else
int             vsnprintf();
#endif

static stralloc sa = {0};
static size_t   subprintf_bufsiz = 128;

int
#ifdef HAVE_STDARG_H
subprintfe(substdio *ss, const char *str, const char *format, ...)
#else
subprintfe(va_alist)
va_dcl
#endif
{
	int             r;
	va_list         ap;
#ifndef HAVE_STDARG_H
	substdio       *ss;
	const char     *ident;
	const char     *format;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, format);
#else
	va_start(ap);
	ss = va_arg(ap, substdio *);
	format = va_arg(ap, const char *);
#endif
	if (!stralloc_ready(&sa, subprintf_bufsiz)) {
		va_end(ap);
		return -1;
	}
	sa.len = 0;
	r = vsnprintf(sa.s, subprintf_bufsiz + 1, format, ap);
	if (r > subprintf_bufsiz) {
		va_end(ap);
		if (!stralloc_ready(&sa, r + 1))
			strerr_die1x(111, "out of memory");
		va_start(ap, format);
		r = vsnprintf(sa.s, r + 1, format, ap);
	}
	sa.len = r + 1;
	va_end(ap);
	r = substdio_put(ss, sa.s, sa.len - 1);
	if (r == -1)
		strerr_die2sys(111, str, ": ");
	return r;
}
