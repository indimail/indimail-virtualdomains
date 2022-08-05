/*
 * $Log: gsasl_mkpasswd.h,v $
 * Revision 1.1  2022-08-05 20:58:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GSASL_MKPASSWD
#define _GSASL_MKPASSWD
#include <stralloc.h>

int             gsasl_mkpasswd(int, char *, int, char *, char *, stralloc *);

#endif
