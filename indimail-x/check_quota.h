/*
 * $Log: check_quota.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * check_quota.h
 *
 */
#ifndef CHECK_QUOTA_H
#define CHECK_QUOTA_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifdef USE_MAILDIRQUOTA
mdir_t          check_quota(char *, mdir_t *);
#else
mdir_t          check_quota(char *);
#endif

#endif
