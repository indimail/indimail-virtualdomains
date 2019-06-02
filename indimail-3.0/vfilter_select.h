/*
 * $Log: vfilter_select.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfilter_select.h
 *
 */
#ifndef VFILTER_SELECT_H
#define VFILTER_SELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef VFILTER
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

int             vfilter_select(char *, int *, stralloc *, int *, int *, stralloc *, stralloc *, int *, stralloc *);
#endif

#endif
