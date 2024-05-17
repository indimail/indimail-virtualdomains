/*
 * $Log: dblock.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dblock.h
 *
 */
#ifndef DBLOCK_H
#define DBLOCK_H

int             getDbLock(const char *, const char);
int             delDbLock(int, const char *, char);
int             readPidLock(const char *, char);
#endif
