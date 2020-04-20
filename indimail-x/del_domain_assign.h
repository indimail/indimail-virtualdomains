/*
 * $Log: del_domain_assign.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * del_domain_assign.h
 *
 */
#ifndef DEL_DOMAIN_ASSIGN_H
#define DEL_DOMAIN_ASSIGN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             del_domain_assign(char *, char *, gid_t, gid_t);

#endif
