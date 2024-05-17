/*
 * $Log: sql_insertaliasdomain.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_insertaliasdomain.h
 *
 */
#ifndef SQL_INSERTALIASDOMAIN_H
#define SQL_INSERTALIASDOMAIN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             sql_insertaliasdomain(const char *, const char *);
#endif

#endif
