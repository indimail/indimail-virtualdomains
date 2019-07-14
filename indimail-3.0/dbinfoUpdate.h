/*
 * $Log: dbinfoUpdate.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dbinfoUpdate.h
 *
 */
#ifndef DBINFOUPDATE_H
#define DBINFOUPDATE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             dbinfoUpdate(char *, int, char *, char *, int, int, char *, char *, char *);
#endif

#endif
