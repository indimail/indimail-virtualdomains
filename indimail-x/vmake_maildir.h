/*
 * $Log: vmake_maildir.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vmake_maildir.h
 *
 */
#ifndef VMAKE_MAILDIR_H
#define VMAKE_MAILDIR_H

int             vmake_maildir(char *, uid_t, gid_t, const char *);

#endif
