/*
 * $Log: valias_delete.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * valias_delete.h
 *
 */
#ifndef VALIAS_DELETE_H
#define VALIAS_DELETE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef VALIAS

int             valias_delete(const char *, const char *, const char *);

#endif
#endif
