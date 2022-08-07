/*
 * $Log: gsasl_mkpasswd.h,v $
 * Revision 1.2  2022-08-07 20:38:54+05:30  Cprogrammer
 * added gsasl_mkpasswd_err()
 *
 * Revision 1.1  2022-08-05 20:58:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GSASL_MKPASSWD
#define _GSASL_MKPASSWD
#include <stralloc.h>

#define DEFAULT_SALT_SIZE 12
#define USAGE_ERR  1
#define MEMORY_ERR 2
#define SODIUM_ERR 3
#define GSASL_ERR  4
#define NO_ERR     0

int             gsasl_mkpasswd(int, char *, int, char *, char *, stralloc *);
const char     *gsasl_mkpasswd_err(int);

#endif
