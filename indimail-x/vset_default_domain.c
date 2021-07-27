/*
 * $Log: vset_default_domain.c,v $
 * Revision 1.3  2021-07-27 18:23:27+05:30  Cprogrammer
 * changed interface to return default domain
 *
 * Revision 1.2  2020-04-01 18:59:10+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-14 21:50:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <getEnvConfig.h>
#endif
#include "ip_map.h"
#include "host_in_locals.h"
#include "vset_default_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: vset_default_domain.c,v 1.3 2021-07-27 18:23:27+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vset_default_domain: out of memory", 0);
	_exit(111);
}

static stralloc defDomain = {0};
/*
 * Set the default domain from either the DEFAULT_DOMAIN or ip_map
 */
char *
vset_default_domain()
{
	char           *p;
#ifdef IP_ALIAS_DOMAINS
	static stralloc host = {0};
#endif

	if (defDomain.len)
		return defDomain.s;
	if ((p = (char *) env_get("INDIMAIL_DOMAIN"))) {
		if (!stralloc_copys(&defDomain, p) ||
				!stralloc_0(&defDomain))
			die_nomem();
		defDomain.len--;
		return defDomain.s;
	}
#ifdef IP_ALIAS_DOMAINS
	p = (char *) env_get("TCPLOCALIP");
	/* courier-imap uses IPv6 */
	if (p &&  *p == ':') {
		for (p += 2; *p != ':'; p++)
		++p;
	}
	if (vget_ip_map(p, &host) == 0 && !host_in_locals(host.s)) {
		if (host.len) {
			if (!stralloc_copy(&defDomain, &host) ||
					!stralloc_0(&defDomain))
				die_nomem();
			defDomain.len--;
		}
		return defDomain.s;
	}
#endif
	if (!defDomain.len) {
		getEnvConfigStr(&p, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		if (!stralloc_copys(&defDomain, p) ||
				!stralloc_0(&defDomain))
			die_nomem();
		defDomain.len--;
	}
	return defDomain.s;
}
