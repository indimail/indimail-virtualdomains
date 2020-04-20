/*
 * $Log: makeseekable.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * makeseekable.h
 *
 */
#ifndef MAKESEEKABLE_H
#define MAKESEEKABLE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MAKE_SEEKABLE
int             makeseekable(int);
#endif

#endif
