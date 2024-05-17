/*
 * $Log: sql_active.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sql_active.h
 *
 */
#ifndef SQL_ACTIVE_H
#define SQL_ACTIVE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

int             sql_active(struct passwd *, const char *, int);

#endif
