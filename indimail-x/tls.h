/*
 * $Log: tls.h,v $
 * Revision 1.4  2021-03-04 11:56:25+05:30  Cprogrammer
 * added host argument to match host with common name
 *
 * Revision 1.3  2021-03-03 14:14:06+05:30  Cprogrammer
 * added cafile argument to tls_init()
 *
 * Revision 1.2  2021-03-03 14:00:54+05:30  Cprogrammer
 * fixed data types
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * tls.h
 *
 */
#ifndef TLS_H
#define TLS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

ssize_t         saferead(int, char *, size_t, long);
ssize_t         safewrite(int, char *, size_t, long);
#ifdef HAVE_SSL
int             tls_init(int, char *, char *, char *);
void            ssl_free();
#endif

#endif
