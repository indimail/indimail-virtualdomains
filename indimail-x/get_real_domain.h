/*
 * $Log: get_real_domain.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * get_real_domain.h
 *
 */
#ifndef VGET_REAL_DOMAIN_H
#define VGET_REAL_DOMAIN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

const char     *get_real_domain(const char *);
#ifdef QUERY_CACHE
void            get_real_domain_cache(char);
#endif

#endif
