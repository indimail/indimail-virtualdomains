/*
 * $Log: getuidgid.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * getuidgid.h
 *
 */
#ifndef GETUIDGID_H
#define GETUIDGID_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include "sys/types.h"
#endif

#define INDIMAILUSER "indimail"

int             getuidgid(uid_t *, gid_t *);

#endif
