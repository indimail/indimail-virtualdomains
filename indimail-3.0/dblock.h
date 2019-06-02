/*
 * $Log: dblock.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dblock.h
 *
 */
#ifndef DBLOCK_H
#define DBLOCK_H

int             getDbLock(char *, char);
int             delDbLock(int, char *, char);
int             readPidLock(char *, char);
#endif
