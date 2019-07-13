/*
 * $Log: vpriv.h,v $
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vpriv.h
 *
 */
#ifndef VPRIV_H
#define VPRIV_H

char           *vpriv_select(char **, char **);
int             vpriv_insert(char *, char *, char *);
int             vpriv_update(char *, char *, char *);
int             vpriv_delete(char *, char *);
#endif
