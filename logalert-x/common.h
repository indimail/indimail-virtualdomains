/*
 * $Log: common.h,v $
 * Revision 1.2  2021-03-15 11:31:30+05:30  Cprogrammer
 * removed unused functions that are now part of libindimail
 *
 * Revision 1.1  2013-05-15 00:13:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef COMMON_H
#define COMMON_H
#ifdef HAVE_CONFIG_H
#include "config.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int64_t mdir_t;
typedef uint64_t umdir_t;
#else
typedef long long mdir_t;
typedef unsigned long long umdir_t;
#endif

#endif /*- #ifdef HAVE_CONFIG_H */
#endif /*- #ifdef COMMON_H */
