/*
 * $Log: sql_getall.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sql_getall.h
 *
 */
#ifndef SQL_GETALL_H
#define SQL_GETALL_H

struct passwd *sql_getall(const char *, int, int);

#endif
