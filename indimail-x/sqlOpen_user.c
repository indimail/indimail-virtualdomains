/*
 * $Log: sqlOpen_user.c,v $
 * Revision 1.3  2021-01-26 00:29:08+05:30  Cprogrammer
 * renamed sql_init() to in_sql_init() to avoid clash with dovecot sql authentication driver
 *
 * Revision 1.2  2019-05-28 17:42:19+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-18 08:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sqlOpen_user.c,v 1.3 2021-01-26 00:29:08+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <alloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#endif
#include "indimail.h"
#include "findmdahost.h"
#include "dbload.h"
#include "LoadDbInfo.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "sql_init.h"
#include "variables.h"
#include "load_mysql.h"

int
sqlOpen_user(const char *email, int connect_all)
{
	int             count, total = 0;
	static stralloc user = {0}, domain = {0}, mdahost = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *ptr, *cptr;
	const char     *real_domain;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;

	if (!(ptr = findmdahost(email, &total))) /*- returns domain:host:port */
		return (-1);
	for (; *ptr && *ptr != ':'; ptr++);
	if (!*ptr++)
		return (-1);
	cptr = ptr;
	for (count = 0; *ptr && *ptr != ':';count++, ptr++);
	if (!stralloc_copyb(&mdahost, cptr, count) || !stralloc_0(&mdahost))
		return (-1);
	mdahost.len--;
	if (connect_all) {
		if (OpenDatabases())
			return (-1);
	} else {
		if (RelayHosts) {
			for (count = 0, rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++, count++);
			total = count;
		} else {
			total = 0;
			if (!(RelayHosts = LoadDbInfo_TXT(&total))) {
				strerr_warn1("sqlOpen_user: LoadDbInfo_TXT: ", &strerr_sys);
				return (-1);
			}
			for (rhostsptr = RelayHosts; *rhostsptr; rhostsptr++)
				(*rhostsptr)->fd = -1;
		}
		if (!MdaMysql) {
			if (!(MdaMysql = (MYSQL **) alloc(sizeof(MYSQL *) * (total)))) {
				strnum1[fmt_ulong(strnum1, total * (int) sizeof(MYSQL *))] = 0;
				strerr_warn3("sqlOpen_user: alloc: ", strnum1, " Bytes: ", &strerr_sys);
				return (-1);
			}
			for (rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++)
				*mysqlptr = (MYSQL *) 0;
		}
	}
	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	for (count = 1, rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++, count++) {
		if (!str_diffn(real_domain, (*rhostsptr)->domain, DBINFO_BUFF) &&
				!str_diffn(mdahost.s, (*rhostsptr)->mdahost, mdahost.len + 1))
			break;
	}
	if (*rhostsptr) {
		if (!connect_all && !*mysqlptr && !((*mysqlptr) = (MYSQL *) alloc(sizeof(MYSQL)))) {
			strnum1[fmt_uint(strnum1, (int) sizeof(MYSQL))] = 0;
			strerr_warn3("sqlOpen_user: alloc: ", strnum1, " Bytes: ", &strerr_sys);
			(*rhostsptr)->fd = -1;
			return (-1);
		}
		if ((*rhostsptr)->fd == -1) {
			if (connect_db(rhostsptr, mysqlptr)) {
				strnum1[fmt_uint(strnum1, count)] = 0;
				strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
				strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " failed db ", (*rhostsptr)->database,
					"@", (*rhostsptr)->server, " for user ", (*rhostsptr)->user, " port ", strnum2, 0);
				(*rhostsptr)->fd = -1;
				return (-1);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		if (in_mysql_ping(*mysqlptr)) {
			strnum1[fmt_uint(strnum1, count)] = 0;
			strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
			strerr_warn12("mysql_ping: (", (char *) in_mysql_error(*mysqlptr), ") ",
				(*rhostsptr)->domain, ": Reconnecting... ", (*rhostsptr)->database, "@",
				(*rhostsptr)->server, " user ", (*rhostsptr)->user, " port ", strnum2, 0);
			in_mysql_close(*mysqlptr);
			if (connect_db(rhostsptr, mysqlptr)) {
				strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " failed db ",
					(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
					(*rhostsptr)->user, " port ", strnum2, 0);
				(*rhostsptr)->fd = -1;
				return (-1);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		in_sql_init(1, *mysqlptr);
		return (0);
	} else
		userNotFound = 1;
	return (-1);
}
#endif
