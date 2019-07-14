/*
 * $Log: r_mkdir.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * r_mkdir.h
 *
 */
#ifndef R_MKDIR_H
#define R_MKDIR_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             r_mkdir(char *, mode_t, uid_t, gid_t);

#endif /*- R_MKDIR_H */
