/*
 * $Log: post_handle.c,v $
 * Revision 1.3  2023-01-22 10:33:51+05:30  Cprogrammer
 * include post_handle.h for function prototype
 *
 * Revision 1.2  2019-04-22 23:14:24+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-14 18:29:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <strerr.h>
#include "indimail.h"
#include "runcmmd.h"
#include "post_handle.h"

#ifndef	lint
static char     sccsid[] = "$Id: post_handle.c,v 1.3 2023-01-22 10:33:51+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
int
post_handle(const char *fmt, ...)
#else
#include <varargs.h>
int
post_handle(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
#endif
	va_list         ap;
	char           *ptr, *cptr;
	int             ret;

#ifndef HAVE_STDARG_H
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		if (vasprintf(&ptr, fmt, ap) == -1)
			strerr_die1sys(111, "post_handle: vasprintf: ");
		va_end(ap);
		if ((cptr = strchr(ptr, ' '))) {
			*cptr = 0;
			if (access(ptr, F_OK)) {
				free(ptr);
				return (0);
			}
			*cptr = ' ';
		} else
		if (access(ptr, F_OK)) {
			free(ptr);
			return (0);
		}
		ret = runcmmd(ptr, 0);
		free(ptr);
		return (ret);
	}
	return (0);
}
