#ifndef COMMON_H
#define COMMON_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void            out(char *, char *);
void            flush(char *);
void            errout(char *, char *);
void            errflush(char *);

#endif /*- COMMON_H */
