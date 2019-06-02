/*
 * $Log: update_quota.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * update_quota.h
 *
 */
#ifndef UPDATE_QUOTA_H
#define UPDATE_QUOTA_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             update_quota(char *, mdir_t);

#endif
