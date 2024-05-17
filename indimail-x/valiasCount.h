/*
 * $Log: valiasCount.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * valiasCount.h
 *
 */
#ifndef VALIASCOUNT_H
#define VALIASCOUNT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef VALIAS
long            valiasCount(const char *, const char *);
#endif

#endif
