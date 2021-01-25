/*
 * $Log: UserInLookup.c,v $
 * Revision 1.3  2021-01-26 00:29:14+05:30  Cprogrammer
 * renamed sql_init() to in_sql_init() to avoid clash with dovecot sql authentication driver
 *
 * Revision 1.2  2019-05-28 17:42:25+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-18 07:56:38+05:30  Cprogrammer
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
#include <str.h>
#include <fmt.h>
#endif
#include "parse_email.h"
#include "get_real_domain.h"
#ifdef CLUSTERED_SITE
#include "sqlOpen_user.h"
#else
#include "iopen.h"
#endif
#include "is_distributed_domain.h"
#include "variables.h"
#include "dbload.h"
#include "sql_init.h"
#include "sql_getpw.h"
#include "valiasCount.h"
#include "variables.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: UserInLookup.c,v 1.3 2021-01-26 00:29:14+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 *  0: User is fine
 *  1: User is not present
 *  2: User is Inactive
 *  3: User is overquota
 * -1: System Error
 */
int
UserInLookup(char *email)
{
	static stralloc user = {0}, domain = {0};
	char           *real_domain;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
#ifdef VALIAS
	int             valias_count;
#endif
#ifdef CLUSTERED_SITE
	int             count;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
#endif

	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 1))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound) { /*- Maybe user is an alias */
#ifdef CLUSTERED_SITE
#ifdef VALIAS
			/*-
			 * No need of checking further
			 * valias uses addusecntrl() to add aliases
			 * to hostcntrl. so if alias not found in hostcntrl,
			 * it will not be present on the mailstore's alias table
			 */
			if ((count = is_distributed_domain(real_domain)) == -1) {
				strerr_warn2(real_domain, ": is_distributed_domain failed", 0);
				return (-1);
			} else
			if (count) {
				is_open = 0;
				return (1);
			}
#endif
			if (OpenDatabases())
				return (-1);
			for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts; *rhostsptr; mysqlptr++, rhostsptr++) {
				if (!str_diffn((*rhostsptr)->domain, real_domain, DBINFO_BUFF)) {
					if ((*rhostsptr)->fd == -1) {
						if (connect_db(rhostsptr, mysqlptr)) {
							strnum1[fmt_uint(strnum1, count)] = 0;
							strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
							strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " Failed db ",
								(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
								(*rhostsptr)->user, " port ", strnum2, 0);
							(*rhostsptr)->fd = -1;
							is_open = 0;
							userNotFound = 0;
							return (-1);
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
							is_open = 0;
							userNotFound = 0;
							strnum1[fmt_uint(strnum1, count)] = 0;
							strerr_warn11(strnum1, ": ", (*rhostsptr)->domain, " Failed db ",
								(*rhostsptr)->database, "@", (*rhostsptr)->server, " for user ",
								(*rhostsptr)->user, " port ", strnum2, 0);
							(*rhostsptr)->fd = -1;
							return (-1);
						} else
							(*rhostsptr)->fd = (*mysqlptr)->net.fd;
					}
#ifdef VALIAS
					in_sql_init(1, *mysqlptr);
					if ((valias_count = valiasCount(user.s, real_domain)) == -1) {
						is_open = 0;
						userNotFound = 0;
						return (-1);
					} else if (valias_count > 0) {
						is_open = 0;
						return (4);
					}
#endif
				}
			} /*- for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++) */
			is_open = 0;
#else /*- #ifdef CLUSTERED_SITE */
#ifdef VALIAS
			if ((valias_count = valiasCount(user.s, real_domain)) == -1)
				return (-1);
			else
			if (valias_count > 0)
				return (4);
#endif
#endif /*- #ifdef CLUSTERED_SITE */
			return (1);
		} else /*- if (userNotFound) */
			return (-1);
	}
	if (!sql_getpw(user.s, real_domain)) {
		if (userNotFound) {
#ifdef VALIAS
			if ((valias_count = valiasCount(user.s, real_domain)) == -1) {
#ifdef CLUSTERED_SITE
				is_open = 0;
#endif
				userNotFound = 0;
				return (-1);
			} else
			if (valias_count > 0) {
#ifdef CLUSTERED_SITE
				is_open = 0;
#endif
				return (4);
			}
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
#endif
			return (1);
		} else {
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	if (is_inactive)
		return (2);
	else
	if (is_overquota)
		return (3);
	return (0);
}
