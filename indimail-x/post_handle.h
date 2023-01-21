#ifndef POST_HANDLE_H
#define POST_HANDLE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDARG_H
int             post_handle(const char *fmt, ...)
					__attribute__ ((format (printf, 1, 2)));
#else
int             post_handle();
#endif

#endif /*- POST_HANDLE_H */
