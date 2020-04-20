/*
 * $Log: vfilter_insert.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfilter_insert.h
 *
 */
#ifndef VFILTER_INSERT_H
#define VFILTER_INSERT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef VFILTER
int             vfilter_insert(char *, char *, int, int, char *, char *, int, char *);
#endif

#endif
