/*
 * $Log: clear_open_smtp.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * clear_open_smtp.h
 *
 */
#ifndef CLEAR_OPEN_SMTP_H
#define CLEAR_OPEN_SMTP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

int             clear_open_smtp(time_t, int);

#endif
