/** \file starttls.c - collect common TLS functionality
 * \author Matthias Andree
 * \date 2006
 */

#include "fetchmail.h"

#include <stdbool.h>
#include <string.h>
#include <strings.h>

/** return true if user allowed opportunistic STARTTLS/STLS */
bool maybe_starttls(struct query *ctl) {
#ifdef SSL_ENABLE
         /* opportunistic  or forced TLS */
    return (!ctl->sslproto || strlen(ctl->sslproto))
	&& !ctl->use_ssl;
#else
    (void)ctl;
    return false;
#endif
}

/** return true if user requires STARTTLS/STLS, note though that this
 * code must always use a logical AND with maybe_tls(). */
bool must_starttls(struct query *ctl) {
#ifdef SSL_ENABLE
    return maybe_starttls(ctl)
	&& (ctl->sslfingerprint || ctl->sslcertck
		|| (ctl->sslproto && ctl->sslproto[0]));
#else
    (void)ctl;
    return false;
#endif
}
