/*
 * $Log: vfilter_update.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfilter_update.h
 *
 */
#ifndef VFILTER_UPDATE_H
#define VFILTER_UPDATE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef VFILTER
int             vfilter_update(char *, int, int, int, char *, char *, int, char *);
#endif

#endif
