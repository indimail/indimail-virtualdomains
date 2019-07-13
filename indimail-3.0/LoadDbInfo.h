/*
 * $Log: LoadDbInfo.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * LoadDbInfo.h
 *
 */
#ifndef LOADDBINFO_H
#define LOADDBINFO_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

DBINFO        **LoadDbInfo_TXT(int *);
int             writemcdinfo(DBINFO **, time_t);

#endif /*- LOADDBINFO_H */
