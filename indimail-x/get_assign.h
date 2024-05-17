#ifndef GET_ASSIGN_H
#define GET_ASSIGN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

char           *get_assign(const char *, stralloc *, uid_t *, gid_t *);
#ifdef QUERY_CACHE
void            get_assign_cache(char);
#endif

#endif /*- GET_ASSIGN_H */
