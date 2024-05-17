/*
 * $Log: sql_passwd.h,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2022-08-05 21:16:08+05:30  Cprogrammer
 * added scram argument to update scram password
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_passwd.h
 *
 */
#ifndef SQL_PASSWD_H
#define SQL_PASSWD_H

int             sql_passwd(const char *, const char *, const char *, const char *);

#endif
