/*
 * $Log: get_Mplexdir.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * get_Mplexdir.h
 *
 */
#ifndef GET_MPLEXDIR_H
#define GET_MPLEXDIR_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

char           *get_Mplexdir(const char *, const char *, int, uid_t, gid_t);

#endif
