/*
 * xmalloc.c -- allocate space or die 
 *
 * Copyright 1998 by Eric S. Raymond.
 * For license terms, see the file COPYING in this directory.
 */

#include "config.h"
#include "fetchmail.h"

#include "xmalloc.h"
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include  <stdlib.h>
#include "i18n.h"

void *xmalloc (size_t n)
{
    void *p;

    p = (void *) malloc(n);
    if (p == (void *) 0)
    {
	report(stderr, GT_("malloc failed\n"));
	abort();
    }
    return(p);
}

void *xrealloc (void *p, size_t n)
{
    if (p == 0)
	return xmalloc (n);
    p = (void *) realloc(p, n);
    if (p == (void *) 0)
    {
	report(stderr, GT_("realloc failed\n"));
	abort();
    }
    return p;
}

char *xstrdup(const char *s)
{
    char *p;
    p = (char *) xmalloc(strlen(s)+1);
    strcpy(p,s);
    return p;
}

char *xstrndup(const char *s, size_t len)
{
    char *p;
    size_t l = strlen(s);

    if (len < l) l = len;
    p = (char *)xmalloc(l + 1);
    memcpy(p, s, l);
    p[l] = '\0';
    return p;
}

/* xmalloc.c ends here */
