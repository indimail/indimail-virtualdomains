/*
 * $Log: PwdInLookup.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * PwdInLookup.h
 *
 */
#ifndef PWDINLOOKUP_H
#define PWDINLOOKUP_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

struct passwd  *PwdInLookup(const char *);

#endif
