/*
 * $Log: vsmtp_delete.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vsmtp_delete.h
 *
 * Revision 1.1  2019-04-11 08:55:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef VSMTP_DELETE_H
#define VSMTP_DELETE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CLUSTERED_SITE
int             vsmtp_delete(char *, char *, char *, int);
#endif

#endif
