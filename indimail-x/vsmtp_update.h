/*
 * $Log: vsmtp_update.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vsmtp_update.h
 *
 * Revision 1.1  2019-04-11 08:55:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef VSMTP_UPDATE_H
#define VSMTP_UPDATE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             vsmtp_update(char *, char *, char *, int, int);
#endif

#endif
