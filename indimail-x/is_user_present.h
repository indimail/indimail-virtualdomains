#ifndef IS_USER_PRESENT_H
#define IS_USER_PRESENT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int             is_user_present(const char *, const char *);
#ifdef QUERY_CACHE
void            is_user_present_cache(char);
#endif

#endif
