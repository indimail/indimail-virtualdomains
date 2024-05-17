/*
 * $Log: smtp_port.h,v $
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * smtp_port.h
 *
 */
#ifndef GET_SMTP_SERVICE_PORT_H
#define GET_SMTP_SERVICE_PORT_H

int             smtp_port(const char *, const char *, const char *);

#endif
