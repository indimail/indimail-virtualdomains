/*
 * $Log: set_mysql_options.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * set_mysql_options.h
 *
 */
#ifndef SET_MYSQL_OPTION_H
#define SET_MYSQL_OPTION_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include <mysql.h>

int             set_mysql_options(MYSQL *, char *, char *, unsigned int *);
int             int_mysql_options(MYSQL *, enum mysql_option, const void *);
char           *error_mysql_options_str(unsigned int);

#endif /*- SET_MYSQL_OPTION_H */
