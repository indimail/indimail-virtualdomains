/*
 * $Log: vget_lastauth.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vget_lastauth.h
 *
 */
#ifndef VGET_LASTAUTH_H
#define VGET_LASTAUTH_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

time_t          vget_lastauth(struct passwd *, const char *, int, char *);

#endif
