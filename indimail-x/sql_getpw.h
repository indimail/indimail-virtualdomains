/*
 * $Log: sql_getpw.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_getpw.h
 *
 */
#ifndef SQL_GETPW_H
#define SQL_GETPW_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

struct passwd  *sql_getpw(const char *, const char *);
#ifdef QUERY_CACHE
void            sql_getpw_cache(char);
#endif

#endif
