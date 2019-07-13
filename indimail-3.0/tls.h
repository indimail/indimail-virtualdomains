/*
 * $Log: tls.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * tls.h
 *
 */
#ifndef TLS_H
#define TLS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

ssize_t         saferead(int, char *, int, int);
ssize_t         safewrite(int, char *, int, int);
#ifdef HAVE_SSL
int             tls_init(int, char *);
void            ssl_free();
#endif

#endif
