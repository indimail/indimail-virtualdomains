#ifndef COMMON_H
#define COMMON_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <substdio.h>

void            out(char *, char *);
void            flush(char *);
void            errout(char *, char *);
void            errflush(char *);
#ifdef HAVE_STDARG_H
int             subprintfe(substdio *ss, const char *ident, const char *__restrict __format, ...)
					__attribute__ ((format (printf, 3, 4)));
#else
int             subprintfe();
#endif

#endif /*- COMMON_H */
