/*
 * $Log: filewrt.c,v $
 * Revision 1.1  2019-04-18 08:35:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: filewrt.c,v 1.1 2019-04-18 08:35:53+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
/* function to write to a file */
int
filewrt(int fout, char *fmt, ...)
#else
#include <varargs.h>
int
filewrt(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif
#ifndef HAVE_STDARG_H
	int             fout;
	char           *fmt;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	fout = va_arg(ap, int);	/* file descriptor */
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	for (len = 0, ptr = buf; *ptr++; len++);
	return (write(fout, buf, len) != len ? -1 : len);
}
