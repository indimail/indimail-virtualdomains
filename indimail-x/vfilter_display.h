/*
 * $Log: vfilter_display.h,v $
 * Revision 1.2  2023-01-22 10:32:09+05:30  Cprogrammer
 * added print_hyphen function
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfilter_display.h
 *
 */
#ifndef VFILTER_DISPLAY_H
#define VFILTER_DISPLAY_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef VFILTER
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <substdio.h>
#endif

int             vfilter_display(char *, int);
void            print_hyphen(substdio *, char *, int);
void            format_filter_display(int, int, char *, stralloc *, int, int, stralloc *, stralloc *, stralloc *, int);

#endif
#endif
