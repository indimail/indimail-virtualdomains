/*
 * $Log: relay_select.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * relay_select.h
 *
 */
#ifndef RELAY_SELECT_H
#define RELAY_SELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef POP_AUTH_OPEN_RELAY
int             relay_select(const char *, const char *);
#endif

#endif
