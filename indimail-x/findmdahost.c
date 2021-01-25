/*
 * $Log: findmdahost.c,v $
 * Revision 1.3  2021-01-26 00:28:43+05:30  Cprogrammer
 * renamed sql_init() to in_sql_init() to avoid clash with dovecot sql authentication driver
 *
 * Revision 1.2  2019-05-28 17:39:14+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-18 08:35:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <alloc.h>
#include <fmt.h>
#include <str.h>
#endif
#include "parse_email.h"
#include "sql_init.h"
#include "sql_getpw.h"
#include "dbload.h"
#include "LoadDbInfo.h"
#include "get_real_domain.h"
#include "findhost.h"
#include "smtp_port.h"
#include "GetSMTProute.h"
#include "is_distributed_domain.h"
#include "variables.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: findmdahost.c,v 1.3 2021-01-26 00:28:43+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <errno.h>

char           *
findmdahost(char *email, int *total)
{
	int             is_dist, count, port, connect_all, i;
	static stralloc user = {0}, domain = {0}, mailhost = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *real_domain, *ip;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	struct passwd *pw;

	if (parse_email(email, &user, &domain))
		return ((char *) 0);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	if ((is_dist = is_distributed_domain(real_domain)) == -1) {
		strerr_warn2(real_domain, ": is_distributed_domain failed", 0);
		return ((char *) 0);
	}
	if (is_dist) {
		if (!(ip = findhost(email, 0)))
			return ((char *) 0);
		else
			return (ip);
	} 
	/*- reach here if non-distributed */
	if (!total)
		connect_all = 1;
	else
		connect_all = *total;
	if (connect_all) {
		if (OpenDatabases())
			return ((char *) 0);
	} else {
		if (RelayHosts) {
			for (count = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++, count++);
			*total = count;
		} else {
			if (!(RelayHosts = LoadDbInfo_TXT(total))) {
				strerr_warn1("findmdahost: LoadDbInfo_TXT", 0);
				return ((char *) 0);
			}
			for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
				(*rhostsptr)->fd = -1;
		}
		if (!MdaMysql) {
			if (!(MdaMysql = (MYSQL **) alloc(sizeof(MYSQL *) * (*total)))) {
				strnum1[fmt_uint(strnum1, *total * (int) sizeof(MYSQL *))] = 0;
				strerr_warn3("findmdahost: alloc: ", strnum1, " Bytes: \n", &strerr_sys);
				return ((char *) 0);
			}
			for (mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
				*mysqlptr = (MYSQL *) 0;
		}
	}
	for (count= 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++, count++) {
		/*- for non distributed only one entry is there in mcd file */
		if (!str_diffn(real_domain, (*rhostsptr)->domain, DBINFO_BUFF))
			break;
	}
	if (*rhostsptr) {
		if (!connect_all && !*mysqlptr && !((*mysqlptr) = (MYSQL *) alloc(sizeof(MYSQL)))) {
			strnum1[fmt_uint(strnum1, (int) sizeof(MYSQL))] = 0;
			strerr_warn3("findmdahost: alloc: ", strnum1, " Bytes: ", &strerr_sys);
			(*rhostsptr)->fd = -1;
			return ((char *) 0);
		}
		if ((*rhostsptr)->fd == -1) {
			if (connect_db(rhostsptr, mysqlptr)) {
				strnum1[fmt_uint(strnum1, count)] = 0;
				strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
				strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " failed db ",
					(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
					(*rhostsptr)->user, " port ", strnum2, 0);
				(*rhostsptr)->fd = -1;
				return ((char *) 0);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		if (in_mysql_ping(*mysqlptr)) {
			strnum1[fmt_uint(strnum1, count)] = 0;
			strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
			strerr_warn12("mysql_ping: (", (char *) in_mysql_error(*mysqlptr), ") ",
				(*rhostsptr)->domain, ": Reconnecting... ", (*rhostsptr)->database,
				"@", (*rhostsptr)->server, " user ", (*rhostsptr)->user, " port ", strnum2, 0);
			in_mysql_close(*mysqlptr);
			if (connect_db(rhostsptr, mysqlptr)) {
				strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " failed db ", (*rhostsptr)->database,
					"@", (*rhostsptr)->server, " for user ", (*rhostsptr)->user, " port ", strnum2, 0);
				(*rhostsptr)->fd = -1;
				return ((char *) 0);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		in_sql_init(1, *mysqlptr);
		if (!(pw = sql_getpw(user.s, real_domain))) {
			is_open = 0; /* prevent closing of connection by iclose */
			return ((char *) 0);
		} else {
			is_open = 0; /* prevent closing of connection by iclose */
			if (is_dist == 1)
				port = smtp_port(0, real_domain, (*rhostsptr)->mdahost);
			else
				port = GetSMTProute(real_domain);
			if (port == -1)
				return ((char *) 0);
			strnum2[i = fmt_uint(strnum2, port)] = 0;
			if (!stralloc_copy(&mailhost, &domain) ||
					!stralloc_append(&mailhost, ":") ||
					!stralloc_cats(&mailhost, (*rhostsptr)->mdahost) ||
					!stralloc_append(&mailhost, ":") ||
					!stralloc_catb(&mailhost, strnum2, i) ||
					!stralloc_0(&mailhost))
				return ((char *) 0);
			return (mailhost.s);
		}
	} else
		userNotFound = 1;
	return ((char *) 0);
}
#endif
