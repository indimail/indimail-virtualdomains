/*
 * $Log: iadduser.h,v $
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2022-11-02 12:44:14+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.3  2022-08-05 22:41:16+05:30  Cprogrammer
 * removed apop argument to iadduser()
 *
 * Revision 1.2  2022-08-05 21:02:29+05:30  Cprogrammer
 * added encrypt_flag argument
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * iadduser.h
 *
 */
#ifndef IADDUSER_H
#define IADDUSER_H

int             iadduser(const char *, const char *, const char *, const char *, const char *, const char *, int, int, int, const char *);

#endif
