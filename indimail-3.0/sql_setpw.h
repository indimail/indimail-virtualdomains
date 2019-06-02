/*
 * $Log: sql_setpw.h,v $
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

int             sql_setpw(struct passwd *, char *);

#endif
