/*
 * $Log: remove_line.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * remove_line.h
 *
 */
#ifndef REMOVE_LINE_H
#define REMOVE_LINE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             remove_line(char *, char *, int, mode_t mode);

#endif
