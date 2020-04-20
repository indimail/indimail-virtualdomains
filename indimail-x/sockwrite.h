/*
 * $Log: sockwrite.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sockwrite.h
 *
 */
#ifndef SOCKWRITE_H
#define SOCKWRITE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

ssize_t         sockwrite(int, char *, int);

#endif
