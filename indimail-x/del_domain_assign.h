/*
 * $Log: del_domain_assign.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
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

int             del_domain_assign(const char *, const char *, gid_t, gid_t);

#endif
