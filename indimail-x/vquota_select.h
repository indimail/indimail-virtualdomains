/*
 * $Log: vquota_select.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vquota_select.h
 *
 */
#ifndef VQUOTA_SELECT_H
#define VQUOTA_SELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

int             vquota_select(stralloc *, stralloc *, mdir_t *, time_t *);

#endif
