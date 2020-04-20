/*
 * $Log: lockfile.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * lockfile.h
 *
 */
#ifndef LOCKFILE_H
#define LOCKFILE_H

int             lockcreate(char *, char);
int             lockremove(int);
int             get_write_lock(int);
int             ReleaseLock(int);
int             RemoveLock(char *, char);

#endif
