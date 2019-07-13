/*
 * $Id: cgi.h,v 1.2 2019-06-03 06:46:17+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef HAVE_CGI_H
#define HAVE_CGI_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

void            get_cgi();
int             GetValue(char *, stralloc *, char *);

#endif
