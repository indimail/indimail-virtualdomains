/*
 * $Log: remove_line.h,v $
 * Revision 1.2  2023-12-03 16:09:37+05:30  Cprogrammer
 * added new function remove_line_p() for partial match
 *
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
int             remove_line_p(char *, char *, int, mode_t mode);

#endif
