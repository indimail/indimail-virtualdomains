/*
 * $Log: lockfile.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * lockfile.h
 *
 */
#ifndef LOCKFILE_H
#define LOCKFILE_H

int             lockcreate(const char *, char);
int             lockremove(int);
int             get_write_lock(int);
int             ReleaseLock(int);
int             RemoveLock(const char *, char);

#endif
