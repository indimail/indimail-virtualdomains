/*
 * $Log: hostcntrl_select.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * hostcntrl_select.h
 *
 */
#ifndef HOSTCNTRL_SELECT_H
#define HOSTCNTRL_SELECT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif
#include <mysql.h>

int             hostcntrl_select(char *, char *, time_t *, stralloc *);
MYSQL_ROW       hostcntrl_select_all();

#endif
