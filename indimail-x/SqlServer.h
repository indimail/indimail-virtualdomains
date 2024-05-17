/*
 * $Log: SqlServer.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * SqlServer.h
 *
 */
#ifndef SQLSERVER_H
#define SQLSERVER_H

char           *SqlServer(const char *, const char *);
char           *MdaServer(const char *, const char *);

#endif
