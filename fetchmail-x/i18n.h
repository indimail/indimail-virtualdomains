#ifndef _I18N_H
#define _I18N_H 42

/* gettext.h is a regular GNU gettext header now */
#include "gettext.h"

/* local modifications */
#define GT_(s) gettext(s)
#define N_(s) gettext_noop(s)

#endif
