/*
 * $Log: inquery.h,v $
 * Revision 1.3  2024-05-28 19:24:35+05:30  Cprogrammer
 * define INFIFODIR for infifo
 *
 * Revision 1.2  2024-05-10 11:43:51+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * inquery.h
 *
 */
#ifndef INQUERY_H
#define INQUERY_H

#define INFIFODIR "/run/indimail/inlookup"

void           *inquery(char, const char *, const char *);

#endif
