/*
 * $Log: open_smtp_relay.c,v $
 * Revision 1.3  2023-04-23 00:47:58+05:30  Cprogrammer
 * record IPv6 address if present in relay table
 *
 * Revision 1.2  2020-04-01 18:57:24+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 16:05:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: open_smtp_relay.c,v 1.3 2023-04-23 00:47:58+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#include <getEnvConfig.h>
#endif
#include "iopen.h"
#include "get_real_domain.h"
#include "skip_relay.h"
#include "create_table.h"
#include "variables.h"
#include "indimail.h"

static void
die_nomem()
{
	strerr_warn1("open_smtp_relay: out of memory", 0);
	_exit(111);
}

/*
 * Gets ipaddr from Env variable TCPREMOTEIP
 * Inserts ipaddr into relay table.
 */
int
open_smtp_relay(char *user, char *domain)
{
	static stralloc SqlBuf = {0};
	int             i;
	char            strnum[FMT_ULONG];
	char           *ipaddr, *relay_table, *real_domain;

#ifdef ENABLE_IPV6
	if (!(ipaddr = env_get("TCP6REMOTEIP")) && !(ipaddr = env_get("TCPREMOTEIP")))
#else
	if (!(ipaddr = env_get("TCPREMOTEIP")))
#endif
		return(0);
	if (skip_relay(ipaddr))
		return(0);
	if (iopen((char *)0))
		return(0);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	if (!(real_domain = get_real_domain(domain)))
		real_domain = domain;
	if (!stralloc_copyb(&SqlBuf, "replace ", 8))
		die_nomem();
	if (delayed_insert && !stralloc_catb(&SqlBuf, "delayed ", 8))
		die_nomem();
	strnum[i = fmt_ulong(strnum, time(0))] = 0;
	if (!stralloc_catb(&SqlBuf, "into ", 5) ||
			!stralloc_cats(&SqlBuf, relay_table) ||
			!stralloc_catb(&SqlBuf, " ( email, ipaddr, timestamp ) values (\"", 39) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "@") ||
			!stralloc_cats(&SqlBuf, real_domain) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, ipaddr) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, i) ||
			!stralloc_append(&SqlBuf, ")") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
				return(0);
			if (!mysql_query(&mysql[1], SqlBuf.s))
				return(1);
		}
		strerr_warn4("open_smtp_relay: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return(0);
	}
	return(1);
}
#endif /*- #ifdef POP_AUTH_OPEN_RELAY */
