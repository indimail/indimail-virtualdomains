#ifndef MD5_H
#define MD5_H

#include "config.h"
#include "fetchmail.h"

#include <stdint.h>
#include <sys/types.h>

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	union {
	    unsigned char in[64];
	    uint32_t	  in32[16];
	} u;
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, const void *buf, unsigned len);
void MD5Final(void *digest, struct MD5Context *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
