/*
 * $Log: get_indimailuidgid.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * get_indimailuidgid.h
 *
 */

#ifndef GET_INDIMAILUIDGID_H
#define GET_INDIMAILUIDGID_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#else
#include <sys/types.h>
#endif

int             get_indimailuidgid(uid_t *, gid_t *);

#endif
