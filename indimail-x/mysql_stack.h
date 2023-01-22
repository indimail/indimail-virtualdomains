/*
 * $Log: mysql_stack.h,v $
 * Revision 1.2  2023-01-22 10:33:13+05:30  Cprogrammer
 * use __attribute__ ((format (printf, 1, 2))) for mysql_stack function prototype
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * mysql_stack.h
 *
 */
#ifndef MYSQL_STACK_H
#define MYSQL_STACK_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

#ifdef HAVE_STDARG_H
char           *mysql_stack __P((const char *, ...))
					__attribute__ ((format (printf, 1, 2)));
#else
char           *mysql_stack ();
#endif

#endif
