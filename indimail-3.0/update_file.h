/*
 * $Log: update_file.h,v $
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

int              update_file(char *, char *, mode_t);

#endif
