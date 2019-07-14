/*
 * $Log: dbinfoSelect.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dbinfoSelect.h
 *
 */
#ifndef DBINFOSELECT_H
#define DBINFOSELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             dbinfoSelect(char *, char *, char *, int);
#endif

#endif
