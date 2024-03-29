/** \file xmalloc.h -- Declarations for the fail-on-OOM string functions */

#ifndef XMALLOC_H
#define XMALLOC_H

#include "config.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* xmalloc.c */

#if !defined __GNUC__ || __GNUC__ < 2
# define __attribute__(xyz)    /* Ignore. */
#endif

/** Allocate \a n characters of memory, abort program on failure. */
void *xmalloc(size_t n) __attribute__((malloc));

/** Reallocate \a n characters of memory, abort program on failure. */
void *xrealloc(/*@null@*/ void *, size_t n);

/** Free memory at position \a p and set pointer \a p to NULL afterwards. */
#define xfree(p) { if (p) { free(p); } (p) = 0; }

/** Duplicate string \a src to a newly malloc()d memory region and return its
 * pointer, abort program on failure. */
char *xstrdup(const char *src);

/** Duplicate at most the first \a n characters from \a src to a newly
 * malloc()d memory region and NUL-terminate it, and return its pointer, abort
 * program on failure. The memory size is the lesser of either the string
 * length including NUL byte or n + 1. */
char *xstrndup(const char *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
