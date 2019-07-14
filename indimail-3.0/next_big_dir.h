/*
 * $Log: next_big_dir.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * next_big_dir.h
 *
 */
#ifndef NEXT_BIG_DIR_H
#define NEXT_BIG_DIR_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

char            *next_big_dir(uid_t, gid_t, int);

#endif
