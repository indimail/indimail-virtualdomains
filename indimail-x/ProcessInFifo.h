/*
 * $Log: ProcessInFifo.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * ProcessInFifo.h
 *
 */
#ifndef PROCESS_INFIFO_H
#define PROCESS_INFIFO_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

int             ProcessInFifo(int);
int             cache_active_pwd(time_t);

#endif
