/*
 * $Log: close_big_dir.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * close_big_dir.h
 *
 */
#ifndef CLOSE_BIG_DIR_H
#define CLOSE_BIG_DIR_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             close_big_dir(const char *, const char *, uid_t, gid_t);

#endif
