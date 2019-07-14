/*
 * $Log: makesalt.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * makesalt.h
 *
 */
#ifndef HAVE_MAKESALT_H
#define HAVE_MAKESALT_H

#define SALTSIZE 32

void            makesalt(char *, int);
char           *genpass(int);

#endif
