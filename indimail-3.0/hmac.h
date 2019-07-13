/*
 * $Log: hmac.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * hmac.h
 *
 */
#ifndef HMAC_H
#define HMAC_H
#include "typesx.h"

void            hmac_md5(unsigned char *, int, unsigned char *, int, void *);
void            hmac_sha1(u8 *, size_t, u8 *, size_t, u8*);
void            hmac_sha256(unsigned char *, size_t, unsigned char *, size_t, void *);
void            hmac_sha512(unsigned char *, size_t, unsigned char *, size_t, void *);
void            hmac_ripemd(u8 *, size_t, u8 *, size_t, u8 *);

#endif
