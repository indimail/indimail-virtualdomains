/*
 * $Log: mgmtpassfuncs.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * mgmtpassfuncs.h
 *
 */
#ifndef MGMTPASSFUNCS_H
#define MGMTPASSFUNCS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

int             getpassword(char *);
int             updateLoginFailed(char *);
int             ChangeLoginStatus(char *, int);
int             mgmtlist();
int             isDisabled(char *);
int             mgmtpassinfo(char *, int);
int             setpassword(char *);
char           *mgmtgetpass(char *, int *);
int             mgmtsetpass(char *, char *, uid_t, gid_t, time_t, time_t);
int             mgmtadduser(char *, char *, uid_t, gid_t, time_t, time_t);

#endif
