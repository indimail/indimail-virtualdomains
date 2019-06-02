/*
 * $Log: mysql_stack.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * mysql_stack.h
 *
 */
#ifndef MYSQL_STACK_H
#define MYSQL_STACK_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDARG_H
char           *mysql_stack __P((char *, ...));
#else
char           *mysql_stack ();
#endif

#endif
