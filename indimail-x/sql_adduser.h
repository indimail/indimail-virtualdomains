/*
 * $Log: sql_adduser.h,v $
 * Revision 1.2  2022-11-02 12:44:34+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sql_adduser.h
 *
 */
#ifndef SQL_ADDUSER_H
#define SQL_ADDUSER_H

char           *sql_adduser(char *, char *, char *, char *, char *, char *, int, int, char *);

#endif
