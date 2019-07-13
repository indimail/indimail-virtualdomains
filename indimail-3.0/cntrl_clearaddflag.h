/*
 * $Log: cntrl_clearaddflag.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * cntrl_clearaddflag.h
 *
 */
#ifndef CNTRL_CLEARADDFLAG_H
#define CNTRL_CLEARADDFLAG_H
#ifdef HAVE_CONFIG_HS
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             cntrl_clearaddflag(char *, char *, char *);
#ifdef QUERY_CACHE
void            cntrl_clearaddflag_cache(char);
#endif
#endif

#endif
