/*
 * $Log: vsmtp_insert.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vsmtp_insert.h
 *
 * Revision 1.1  2019-04-11 08:55:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef VSMTP_INSERT_H
#define VSMTP_INSERT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             vsmtp_insert(char *, char *, char *, int);
#endif

#endif
