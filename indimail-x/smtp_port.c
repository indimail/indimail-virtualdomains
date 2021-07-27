/*
 * $Log: smtp_port.c,v $
 * Revision 1.5  2021-07-27 18:07:11+05:30  Cprogrammer
 * set default domain using vset_default_domain
 *
 * Revision 1.4  2020-04-01 18:57:55+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-06-17 23:26:17+05:30  Cprogrammer
 * set default port as PORT_SMTP
 *
 * Revision 1.2  2019-04-17 17:44:42+05:30  Cprogrammer
 * set default port as PORT_SMTP
 *
 * Revision 1.1  2019-04-14 22:54:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CLUSTERED_SITE
#include <sys/socket.h>
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include <strerr.h>
#include <scan.h>
#endif
#include "indimail.h"
#include "get_local_ip.h"
#include "findhost.h"
#include "create_table.h"
#include "vset_default_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: smtp_port.c,v 1.5 2021-07-27 18:07:11+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("smtp_port: out of memory", 0);
	_exit(111);
}

int
smtp_port(char *srchost, char *domain, char *hostid)
{
	static stralloc Domain = {0}, SqlBuf = {0};
	char           *ptr, *srchost_t;
	int             i;
	int             default_port;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if ((ptr = env_get("ROUTES")) && *ptr) {
		if (!str_diffn(ptr, "smtp", 4))
			default_port = PORT_SMTP;
		else
		if (!str_diffn(ptr, "qmtp", 4))
			default_port = PORT_QMTP;
		else
			default_port = PORT_SMTP;
	} else
		default_port = PORT_SMTP;
	if (open_central_db(0))
		return (default_port);
	if (!domain || !*domain) {
		ptr = vset_default_domain();
		if (!stralloc_copys(&Domain, ptr) || !stralloc_0(&Domain))
			die_nomem();
		Domain.len--;
	} else {
		if (!stralloc_copys(&Domain, domain) || !stralloc_0(&Domain))
			die_nomem();
		Domain.len--;
	}
	if (!srchost || !*srchost) {
		if (!(srchost_t = get_local_ip(PF_INET)))
			return (default_port);
	} else
		srchost_t = srchost;
	if (!stralloc_copyb(&SqlBuf, "select high_priority port, src_host from smtp_port where host=\"", 63) ||
		!stralloc_cats(&SqlBuf, hostid) ||
		!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
		!stralloc_cat(&SqlBuf, &Domain) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
		else {
			strerr_warn4("smtp_port: ", SqlBuf.s, ": ",
				(char *) in_mysql_error(&mysql[0]), 0);
		}
		return (default_port);
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("smtp_port: in_mysql_store_result: ",
				(char *) in_mysql_error(&mysql[0]), 0);
		return (default_port);
	}
	for(i = -1;;) {
		if (!(row = in_mysql_fetch_row(res)))
			break;
		if (!str_diff(srchost_t, row[1])) {
			scan_int(row[0], &i);
			in_mysql_free_result(res);
			return (i ? i : default_port);
		}
		if (*row[1] == '*')
			scan_int(row[0], &i);
	}
	in_mysql_free_result(res);
	if (i == -1)
		return (default_port);
	return (i ? i : default_port);
}
#endif
