/*
 * $Log: VlimitInLookup.h,v $
 * Revision 1.2  2019-04-15 22:00:16+05:30  Cprogrammer
 * added vlimits.h for vlimit struct definition
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * VlimitInLookup.h
 *
 */
#ifndef VLIMITINLOOKUP_H
#define VLIMITINLOOKUP_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vlimits.h"

#ifdef ENABLE_DOMAIN_LIMITS
int             VlimitInLookup(const char *, struct vlimits *);
#endif

#endif
