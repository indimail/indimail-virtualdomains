/*
 * $Log: storeHeader.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * storeHeader.h
 *
 */
#ifndef STOREHEADER_H
#define STOREHEADER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef VFILTER
#include "eps.h"

struct header
{
	char           *name;
	char          **data;
	int             data_items;
};

int             storeHeader(struct header ***, struct header_t *);
#endif

#endif
