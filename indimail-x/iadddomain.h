/*
 * $Log: iadddomain.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * iadddomain.h
 *
 */
#ifndef ADDDOMAIN_H
#define ADDDOMAIN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             iadddomain(char *, char *, char *, uid_t, gid_t, int);

#endif
