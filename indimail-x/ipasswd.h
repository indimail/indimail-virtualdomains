/*
 * $Log: ipasswd.h,v $
 * Revision 1.2  2022-08-05 21:09:06+05:30  Cprogrammer
 * added scram argument for updating scram password
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * ipasswd.h
 *
 */
#ifndef IPASSWD_H
#define IPASSWD_H

int             ipasswd(const char *, const char *, const char *, int, const char *);

#endif
