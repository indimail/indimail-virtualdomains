/*
 * $Log: update_file.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * update_file.h
 *
 */
#ifndef UPDATE_FILE_H
#define UPDATE_FILE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int              update_file(const char *, const char *, mode_t);

#endif
