/*
 * $Log: vfstab.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vfstab.h
 *
 */
#ifndef VFSTAB_HEADER_H
#define VFSTAB_HEADER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

char           *vfstab_select(stralloc *, int *, long *, long *, long *, long *);
int             vfstab_insert(char *, char *, int, long, long);
int             vfstab_delete(char *, char *);
int             vfstab_update(char *, char *, long, long, int);
int             vfstabNew(char *, long, long);
int             vfstab_status(char *, char *, int);

#endif
