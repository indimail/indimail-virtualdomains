/*
 * $Log: sql_init.h,v $
 * Revision 1.2  2021-01-26 00:29:04+05:30  Cprogrammer
 * renamed sql_init() to in_sql_init() to avoid clash with dovecot sql authentication driver
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_init.h
 *
 */
#ifndef SQL_INIT_H
#define SQL_INIT_H
#include <mysql.h>

void            in_sql_init(int, MYSQL *);

#endif
