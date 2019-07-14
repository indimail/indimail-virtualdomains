/*
 * $Log: mdaMySQLConnect.c,v $
 * Revision 1.2  2019-05-28 17:41:20+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-18 08:36:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: mdaMySQLConnect.c,v 1.2 2019-05-28 17:41:20+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_QMAIL
#include <str.h>
#include <strerr.h>
#include <fmt.h>
#endif
#include "dbload.h"
#include "load_mysql.h"

MYSQL **
mdaMySQLConnect(char *mdahost, char *domain)
{
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	int             count;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if(OpenDatabases())
		return((MYSQL **) 0);
	for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++) {
		if (!str_diffn((*rhostsptr)->domain, domain, DBINFO_BUFF) && !str_diffn((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF)) {
			if ((*rhostsptr)->fd == -1) {
				if (connect_db(rhostsptr, mysqlptr)) {
					strnum1[fmt_uint(strnum1, count)] = 0;
					strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
					strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " Failed db ",
							(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
							(*rhostsptr)->user, " port ", strnum2, 0);
					(*rhostsptr)->fd = -1;
					return((MYSQL **) 0);
				} else
					(*rhostsptr)->fd = (*mysqlptr)->net.fd;
			}
			if (in_mysql_ping(*mysqlptr)) {
				strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
				strerr_warn12("mysql_ping: (", (char *) in_mysql_error(*mysqlptr), ") ", (*rhostsptr)->domain,
					": Reconnecting... ", (*rhostsptr)->database, "@", (*rhostsptr)->server,
					" user ", (*rhostsptr)->user, " port ", strnum2, 0);
				in_mysql_close(*mysqlptr);
				if (connect_db(rhostsptr, mysqlptr)) {
					strnum1[fmt_uint(strnum1, count)] = 0;
					strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " Failed db ",
						(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
						(*rhostsptr)->user, " port ", strnum2, 0);
					(*rhostsptr)->fd = -1;
					return((MYSQL **) 0);
				} else
					(*rhostsptr)->fd = (*mysqlptr)->net.fd;
			}
			return(mysqlptr);
		}
	}
	return((MYSQL **) 0);
}
#endif
