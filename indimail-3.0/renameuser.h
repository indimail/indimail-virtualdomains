/*
 * $Log: renameuser.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * renameuser.h
 *
 */
#ifndef RENAMEUSER_H
#define RENAMEUSER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

int             renameuser(stralloc *, stralloc *, stralloc *, stralloc *);

#endif
