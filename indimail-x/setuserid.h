/*
 * $Log: setuserid.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * setuserid.h
 *
 */
#ifndef SETUSERID_H
#define SETUSERID_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

gid_t           *grpscan(char *, int *);
int              setuserid(char *);
int              setuser_privileges(uid_t, gid_t, char *);

#endif
