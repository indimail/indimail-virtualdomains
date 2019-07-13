/*
 * $Log: SqlServer.c,v $
 * Revision 1.3  2019-06-27 16:30:23+05:30  Cprogrammer
 * set ssl parameter in the returned mysql connection string
 *
 * Revision 1.2  2019-04-17 17:47:36+05:30  Cprogrammer
 * use stralloc variable for returning result
 *
 * Revision 1.1  2019-04-14 18:28:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#endif
#include "LoadDbInfo.h"

#ifndef	lint
static char     sccsid[] = "$Id: SqlServer.c,v 1.3 2019-06-27 16:30:23+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("SqlServer: out of memory", 0);
	_exit(111);
}

char *
SqlServer(char *mdahost, char *domain)
{
	DBINFO        **rhostsptr;
	char            strnum[FMT_ULONG];
	int             total;
	static stralloc hostbuf = {0};

	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total))) {
		if (errno != error_noent)
			strerr_warn1("SqlServer: ", &strerr_sys);
		return ((char *) 0);
	}
	for (rhostsptr = RelayHosts; *rhostsptr; rhostsptr++) {
		if (!str_diffn((*rhostsptr)->domain, domain, DBINFO_BUFF) && !str_diffn((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF)) {
	 		/* host:user:password:socket/port:ssl */
			if (!stralloc_copys(&hostbuf, (*rhostsptr)->server) ||
					!stralloc_append(&hostbuf, ":") ||
					!stralloc_cats(&hostbuf, (*rhostsptr)->user) ||
					!stralloc_append(&hostbuf, ":") ||
					!stralloc_cats(&hostbuf, (*rhostsptr)->password))
				die_nomem();
			if ((*rhostsptr)->socket) {
				if (!stralloc_append(&hostbuf, ":") ||
					!stralloc_cats(&hostbuf, (*rhostsptr)->socket))
				die_nomem();
			} else {
				if (!stralloc_append(&hostbuf, ":") ||
					!stralloc_catb(&hostbuf, strnum, fmt_uint(strnum, (*rhostsptr)->port)))
				die_nomem();
			}
			if (!stralloc_cats(&hostbuf, (*rhostsptr)->use_ssl ? ":ssl" : ":nossl"))
				die_nomem();
			if (!stralloc_0(&hostbuf))
				die_nomem();
			return (hostbuf.s);
		}
	}
	return ((char *) 0);
}

char *
MdaServer(char *sqlhost, char *domain)
{
	DBINFO        **rhostsptr;
	int             total;

	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total))) {
		if (errno != error_noent)
			strerr_warn1("SqlServer: ", &strerr_sys);
		return ((char *) 0);
	}
	for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++) {
		if (!str_diffn((*rhostsptr)->domain, domain, DBINFO_BUFF) && !str_diffn((*rhostsptr)->server, sqlhost, DBINFO_BUFF))
			return ((*rhostsptr)->mdahost);
	}
	return ((char *) 0);
}
