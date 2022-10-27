/*
 * $Log: is_user_present.c,v $
 * Revision 1.2  2022-10-27 17:07:22+05:30  Cprogrammer
 * make variables static to avoid clash
 *
 * Revision 1.1  2019-04-20 08:13:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <env.h>
#include <stralloc.h>
#endif
#include "create_table.h"
#include "get_real_domain.h"
#include "variables.h"
#include "findhost.h"

#ifndef	lint
static char     sccsid[] = "$Id: is_user_present.c,v 1.2 2022-10-27 17:07:22+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * -1 - Mysql Error (or) Assignment Error
 *  0 - User Not Present
 *  1 - User Present
 */

#ifdef CLUSTERED_SITE
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static stralloc User = { 0 };
static stralloc Domain = { 0 };
static stralloc SqlBuf = { 0 };

static void
die_nomem()
{
	strerr_warn1("is_user_present: out of memory", 0);
	_exit(111);
}

int
is_user_present(char *user, char *domain)
{
	int             ret;
	static int      is_present;
	char           *real_domain;
	MYSQL_RES      *res;

	if (!user || !*user || !domain || !*domain)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE") && is_present != -1 && 
		!str_diffn(user, User.s, User.len + 1) && !str_diffn(domain, Domain.s, Domain.len + 1))
		return (is_present);
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	real_domain = (char *) 0;
	userNotFound = is_present = 0;
	if (!(real_domain = get_real_domain(domain))) {
		strerr_warn3("is_user_present: ", domain, ": No such domain", 0);
		return (is_present = -1);
	}
	if (open_central_db(0))
		return (is_present = -1);
	if (!stralloc_copyb(&SqlBuf, "select high_priority host from ", 31) ||
		!stralloc_cats(&SqlBuf, cntrl_table) ||
		!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
		!stralloc_cats(&SqlBuf, user) ||
		!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
		!stralloc_cats(&SqlBuf, real_domain) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			userNotFound = 1;
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
				return (is_present = -1);
			return (is_present = 0);
		} else {
			strerr_warn4("is_user_present: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (is_present = -1);
		}
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("is_user_present: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (is_present = -1);
	}
	ret = in_mysql_num_rows(res);
	in_mysql_free_result(res);
	if (!stralloc_copys(&User, user) || !stralloc_0(&User))
		die_nomem();
	User.len--;
	if (!stralloc_copys(&Domain, domain) || !stralloc_0(&Domain))
		die_nomem();
	Domain.len--;
	if (!ret)
		userNotFound = 1;
	return (is_present = ret);
}

#ifdef QUERY_CACHE
void
is_user_present_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif
