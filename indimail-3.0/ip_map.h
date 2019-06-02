/*
 * $Log: ip_map.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * ip_map.h
 *
 */
#ifndef IP_MAP_H
#define IP_MAP_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef IP_ALIAS_DOMAINS
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

int             vget_ip_map(char *, stralloc *);
int             add_ip_map(char *, char *);
int             del_ip_map(char *, char *);
int             upd_ip_map(char *, char *);
int             show_ip_map(int, stralloc *, stralloc *, char *);

#endif
#endif
