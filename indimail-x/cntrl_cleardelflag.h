/*
 * $Log: cntrl_cleardelflag.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * cntrl_cleardelflag.h
 *
 */
#ifndef CNTRL_CLEARDELFLAG_H
#define CNTRL_CLEARDELFLAG_H
#ifdef HAVE_CONFIG_HS
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             cntrl_cleardelflag(char *, char *);
#ifdef QUERY_CACHE
void            cntrl_cleardelflag_cache(char);
#endif
#endif

#endif
