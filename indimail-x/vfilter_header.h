/*
 * $Log: vfilter_header.h,v $
 * Revision 1.2  2023-09-05 21:51:05+05:30  Cprogrammer
 * added prototype for headerNumber function
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfilter_header.h
 *
 */
#ifndef VFILTER_HEADER_H
#define VFILTER_HEADER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef VFILTER
char          **headerList();
int             headerNumber(char **, char *);
#endif

#endif
