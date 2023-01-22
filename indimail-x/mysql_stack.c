/*
 * $Log: mysql_stack.c,v $
 * Revision 1.2  2023-01-22 10:34:48+05:30  Cprogrammer
 * free allocated string
 *
 * Revision 1.1  2019-04-14 21:13:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define _GNU_SOURCE
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif
#include "mysql_stack.h"

static void
die_nomem()
{
	strerr_warn1("mysql_stack: out of memory", 0);
	_exit(111);
}

#ifdef HAVE_STDARG_H
#include <stdarg.h>
char           *
mysql_stack(const char *fmt, ...)
#else
#include <varargs.h>
char           *
mysql_stack(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
#endif
	va_list         ap;
	static stralloc mysqlQueryStr = {0};
	char           *mysqlstr;

#ifndef HAVE_STDARG_H
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		/*- vasprintf is a GNU extension */
		if (vasprintf(&mysqlstr, fmt, ap) == -1) {
			strerr_warn1("mysql_stack: vasprintf: ", &strerr_sys);
			return ((char *) 0);
		}
		va_end(ap);
		if (!stralloc_cats(&mysqlQueryStr, mysqlstr)) {
			free(mysqlstr);
			return ((char *) 0);
		}
		free(mysqlstr);
		return (mysqlQueryStr.s);
	} else {
		if (!mysqlQueryStr.len)
			return ((char *) 0);
		if (mysqlQueryStr.s[mysqlQueryStr.len - 1] != ';' &&
				(!stralloc_append(&mysqlQueryStr, ";") ||
				 !stralloc_0(&mysqlQueryStr)))
			die_nomem();
		mysqlQueryStr.len = 0;
		return (mysqlQueryStr.s);
	}
}
