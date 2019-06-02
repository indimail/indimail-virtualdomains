/*
 * $Id: cgi.h,v 1.1 2010-04-26 12:07:42+05:30 Cprogrammer Exp mbhangui $
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
