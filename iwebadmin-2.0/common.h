#ifndef COMMON_H
#define COMMON_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void            out(char *);
void            flush(void);
void            errout(char *);
void            errflush(void);
void            copy_status_mesg(char *);
void            set_status_mesg_size(int);
void            die_nomem();

#endif /*- COMMON_H */
