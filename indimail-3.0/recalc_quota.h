/*
 * $Log: recalc_quota.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * recalc_quota.h
 *
 */
#ifndef RECALC_QUOTA_H
#define RECALC_QUOTA_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifdef USE_MAILDIRQUOTA
mdir_t          recalc_quota(char *, mdir_t *, mdir_t, mdir_t, int);
#else
mdir_t          recalc_quota(char *, int);
#endif

#endif
