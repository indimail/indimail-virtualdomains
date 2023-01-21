/*
 * $Log: filewrt.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * filewrt.h
 *
 */
#ifndef FILEWRT_H
#define FILEWRT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

#ifdef HAVE_STDARG_H
int             filewrt     __P((int, char *, ...))
					__attribute__ ((format (printf, 2, 3)));
#else
int             filewrt     ();
#endif

#endif
