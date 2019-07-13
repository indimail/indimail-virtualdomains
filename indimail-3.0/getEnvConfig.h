/*
 * $Log: getEnvConfig.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * getEnvConfig.h
 *
 */
#ifndef GETENVCONFIGSTR_H
#define GETENVCONFIGSTR_H

void            getEnvConfigStr(char **, char *, char *);
void            getEnvConfigInt(int *, char *, int);
void            getEnvConfigLong(long *, char *, long);
void            getEnvConfiguLong(unsigned long *, char *, unsigned long);

#endif /*- GETENVCONFIGSTR_H */
