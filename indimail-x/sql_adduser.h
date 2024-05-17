/*
 * $Log: sql_adduser.h,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2022-11-02 12:44:34+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sql_adduser.h
 *
 */
#ifndef SQL_ADDUSER_H
#define SQL_ADDUSER_H

char           *sql_adduser(const char *, const char *, const char *, const char *, const char *, const char *, int, int, const char *);

#endif
