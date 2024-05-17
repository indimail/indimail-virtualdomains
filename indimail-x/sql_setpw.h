/*
 * $Log: sql_setpw.h,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2022-08-07 13:02:50+05:30  Cprogrammer
 * added scram argument to set scram password
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_setpw.h
 *
 */
#ifndef SQL_SETPW_H
#define SQL_SETPW_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

int             sql_setpw(struct passwd *, const char *, const char *);

#endif
