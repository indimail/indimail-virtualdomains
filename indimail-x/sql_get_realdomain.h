/*
 * $Log: sql_get_realdomain.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_get_realdomain.h
 *
 */
#ifndef SQL_GET_REALDOMAIN_H
#define SQL_GET_REALDOMAIN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

char           *sql_get_realdomain(const char *);
#ifdef QUERY_CACHE
void            sql_get_realdomain_cache(char);
#endif

#endif
