/*
 * $Log: parseAddress.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * parseAddress.h
 *
 */
#ifndef PARSEADDRESS_H
#define PARSEADDRESS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif
#include "storeHeader.h"

void            parseAddress(struct header_t *, stralloc *);

#endif
