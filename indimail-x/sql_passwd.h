/*
 * $Log: sql_passwd.h,v $
 * Revision 1.2  2022-08-05 21:16:08+05:30  Cprogrammer
 * added scram argument to update scram password
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * sql_passwd.h
 *
 */
#ifndef SQL_PASSWD_H
#define SQL_PASSWD_H

int             sql_passwd(char *, char *, char *, char *);

#endif
