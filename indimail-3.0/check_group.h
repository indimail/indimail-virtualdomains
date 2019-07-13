/*
 * $Log: check_group.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * check_group.h
 *
 */
#ifndef CHECKGROUP_H
#define CHECKGROUP_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             check_group(gid_t, char *);

#endif /*- ifndef CHECKGROUP_H */
