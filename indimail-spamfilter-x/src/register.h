/* register.h -- constants and declarations for register.c */

#ifndef	REGISTER_H
#define	REGISTER_H

#include "wordhash.h"

extern void register_words(run_t _run_type, wordhash_t *h, u_int32_t msgcount);

#endif	/* REGISTER_H */
