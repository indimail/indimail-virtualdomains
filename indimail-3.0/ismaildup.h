/*
 * $Log: ismaildup.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * ismaildup.h
 *
 * Revision 2.1  2011-06-30 20:37:48+05:30  Cprogrammer
 * prototype for ismaildup()
 *
 */
#ifndef ISMAILDUP_H
#define ISMAILDUP_H
#include "indimail.h"

#ifndef	lint
static char     sccsidisduph[] = "$Id: ismaildup.h,v 1.1 2019-04-13 23:39:27+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_SSL
int             ismaildup(char *);
#endif

#endif
