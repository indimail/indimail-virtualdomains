/*
 * $Log: strToPw.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * strToPw.h
 *
 */
#ifndef STRTOPW_H
#define STRTOPW_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

struct passwd  *strToPw(char *, int);

#endif
