/*
 * $Log: GetSMTProute.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * GetSMTProute.h
 *
 */
#ifndef GETSMTPROUTE_H
#define GETSMTPROUTE_H

int             GetSMTProute(const char *);
int             get_smtp_qmtp_port(const char *, const char *, int);

#endif
