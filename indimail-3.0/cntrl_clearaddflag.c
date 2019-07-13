/*
 * $Log: cntrl_clearaddflag.c,v $
 * Revision 1.1  2019-04-14 21:56:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: cntrl_clearaddflag.c,v 1.1 2019-04-14 21:56:18+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
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
#include <str.h>
#include <fmt.h>
#include <env.h>
#endif
#include "is_user_present.h"
#include "create_table.h"
#include "get_local_hostid.h"
#include "open_master.h"
#include "variables.h"
#include "sql_updateflag.h"

/*
 * -1 - Mysql Error (or) Assignment Error , so insert was a failure
 *  1 - Add in hostcntrl was a success
 */

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static void
die_nomem()
{
	strerr_warn1("cntrl_clearaddflag: out of memory", 0);
	_exit(111);
}

int
cntrl_clearaddflag(char *user, char *domain, char *passwd)
{
	int             ret;
	static int      is_present;
	static stralloc User = {0}, Domain = {0}, SqlBuf = {0};
	char 		   *hostid;
	char            strnum[FMT_ULONG];

	if (!user || !*user || !domain || !*domain || !passwd || !*passwd)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE") && is_present != -1 && !str_diffn(user, User.s, User.len + 1) 
		&& !str_diffn(domain, Domain.s, Domain.len + 1))
		return (is_present);
	else {
		if (!stralloc_copys(&User, user) ||
				!stralloc_0(&User))
			die_nomem();
		User.len--;
		if (!stralloc_copys(&Domain, domain) ||
				!stralloc_0(&Domain))
			die_nomem();
		Domain.len--;
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#else
	if (!stralloc_copys(&User, user) ||
			!stralloc_0(&User))
		die_nomem();
	User.len--;
	if (!stralloc_copys(&Domain, domain) ||
			!stralloc_0(&Domain))
		die_nomem();
	Domain.len--;
#endif
	if (open_master()) {
		strerr_warn1("cntrl_clearaddflag: failed to open master db", 0);
		return (is_present = -1);
	}
	if ((ret = is_user_present(user, domain)) == -1) {
		strerr_warn1("cntrl_clearaddflag: auth db error", 0);
		is_present = -1;
		return (-1);
	}	
	if (ret == 1) {
		is_present = 0;
		return (0);
	}
	if (!(hostid = get_local_hostid())) {
		strerr_warn1("cntrl_clearaddflag: Unable to get Local IPAddress: ", &strerr_sys);
		is_present = -1;
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into ", 25) ||
			!stralloc_cats(&SqlBuf, cntrl_table) ||
			!stralloc_catb(&SqlBuf, " (pw_name, pw_domain, pw_passwd, host, timestamp) values(\"", 58) ||
			!stralloc_cat(&SqlBuf, &User) ||
			!stralloc_catb(&SqlBuf, "\",\"", 3) ||
			!stralloc_cat(&SqlBuf, &Domain) ||
			!stralloc_catb(&SqlBuf, "\",\"", 3) ||
			!stralloc_cats(&SqlBuf, passwd) ||
			!stralloc_catb(&SqlBuf, "\",\"", 3) ||
			!stralloc_cats(&SqlBuf, hostid) ||
			!stralloc_catb(&SqlBuf, "\", FROM_UNIXTIME(", 17) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, time(0))) ||
			!stralloc_catb(&SqlBuf, "))", 2) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT)) {
				strerr_warn4("cntrl_clearaddflag: create_table: ", cntrl_table, ": ", (char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
			if (!mysql_query(&mysql[0], SqlBuf.s))
				return (is_present = (sql_updateflag(user, domain, 1) ? 0 : 1));
		} 
		strerr_warn4("cntrl_clearaddflag: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (is_present = -1);
	}
	return (is_present = (sql_updateflag(user, domain, 1) ? 0 : 1));
}

#ifdef QUERY_CACHE
void
cntrl_clearaddflag_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif
