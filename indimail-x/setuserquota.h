/*
 * $Log: setuserquota.h,v $
 * Revision 1.3  2024-05-24 14:48:31+05:30  Cprogrammer
 * changed return type of setuserquota to 64bit int
 *
 * Revision 1.2  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * setuserquota.h
 *
 */
#ifndef SETUSERQUOTA_H
#define SETUSERQUOTA_H
#include <indimail.h>

mdir_t          setuserquota(const char *, const char *, const char *);

#endif /*- SETUSERQUOTA_H */
