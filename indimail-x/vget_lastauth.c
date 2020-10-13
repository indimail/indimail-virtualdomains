/*
 * $Log: vget_lastauth.c,v $
 * Revision 1.3  2020-10-13 22:49:15+05:30  Cprogrammer
 * null terminate ipaddr
 *
 * Revision 1.2  2019-04-22 23:19:22+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.1  2019-04-14 21:51:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vget_lastauth.c,v 1.3 2020-10-13 22:49:15+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <scan.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "indimail.h"

static void
die_nomem()
{
	strerr_warn1("vget_lastauth: out of memory", 0);
	_exit(111);
}

time_t
vget_lastauth(struct passwd *pw, char *domain, int type, char *ipaddr)
{
	int             err, n;
	time_t          tmval1, tmval2;
	static stralloc SqlBuf = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (ipaddr)
		*ipaddr = 0;
	if ((err = iopen((char *) 0)) != 0)
		return (-1);
	switch (type)
	{
		case AUTH_TIME: /*- Last Authentication */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and (service = \"pop3\" or service=\"imap\" or service=\"webm\")", 60) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case CREAT_TIME: /*- User Creation Date */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"add\"", 21) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case PASS_TIME: /*- Last password change */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"pass\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case ACTIV_TIME: /*- Activation date */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"ACTI\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case INACT_TIME: /*- Inactivation date */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"INAC\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case POP3_TIME: /*- Last POP3 access */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"pop3\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case IMAP_TIME: /*- Last IMAP access */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"imap\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
		case WEBM_TIME: /*- Last WEB access */
			if (!stralloc_copyb(&SqlBuf,
					"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"", 84) ||
					!stralloc_cats(&SqlBuf, pw->pw_name) ||
					!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_catb(&SqlBuf, "\" and service = \"webm\"", 22) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
		break;
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
		else
			strerr_warn4("vget_lastauth: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return(0);
	}
	res = in_mysql_store_result(&mysql[1]);
	tmval1 = 0;
	while ((row = in_mysql_fetch_row(res))) {
		scan_ulong(row[0], (unsigned long *) &tmval2);
		if (tmval2 > tmval1) {
			tmval1 = tmval2;
			if (ipaddr) {
				n = str_copyb(ipaddr, row[1], 16); /*- this can be max 15 bytes length in LASTAUTH definition */
				ipaddr[n] = 0;
			}
		}
	}
	in_mysql_free_result(res);
	return (tmval1);
}
#endif
