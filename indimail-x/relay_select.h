/*
 * $Log: relay_select.h,v $
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
int             relay_select(char *, char *);
#endif

#endif
