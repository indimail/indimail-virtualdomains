/*
 * $Log: valias_select.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * valias_select.h
 *
 */
#ifndef VALIAS_SELECT_H
#define VALIAS_SELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

char           *valias_select(const char *, const char *);
int             valias_track(const char*, stralloc *, stralloc *);
char           *valias_select_all(stralloc *, stralloc *);

#endif
