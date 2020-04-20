/*
 * $Log: cntrl_cleardelflag.c,v $
 * Revision 1.1  2019-04-14 21:57:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: cntrl_cleardelflag.c,v 1.1 2019-04-14 21:57:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <env.h>
#endif
#include "open_master.h"
#include "is_user_present.h"
#include "create_table.h"
#include "sql_updateflag.h"
#include "variables.h"

/*
 * -1 - Mysql Error (or) Assignment Error , so insert was a failure
 *  1 - Delete from Location DB was a success
 */
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static void
die_nomem()
{
	strerr_warn1("cntrl_cleardelflag: out of memory", 0);
	_exit(111);
}

int
cntrl_cleardelflag(char *user, char *domain)
{
	int             ret;
	static stralloc User = {0}, Domain = {0}, SqlBuf = {0};
	static int      is_present;

	if (!user || !*user || !domain || !*domain)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE") && is_present != -1 && 
		!str_diffn(user, User.s, User.len + 1) && !str_diffn(domain, Domain.s, Domain.len + 1))
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
		strerr_warn1("cntrl_cleardelflag: failed to open master db", 0);
		return (is_present = -1);
	}
	if ((ret = is_user_present(user, domain)) == -1) {
		strerr_warn1("cntrl_cleardelflag: auth db error", 0);
		return (is_present = -1);
	}	
	if (ret == 1) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, cntrl_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cat(&SqlBuf, &User) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cat(&SqlBuf, &Domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
				create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			else {
				strerr_warn4("cntrl_cleardelflag: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				return (is_present = -1);
			}
		}
	}
	return (is_present = (sql_updateflag(user, domain, -1) ? 0 : 1));
}

#ifdef QUERY_CACHE
void
cntrl_cleardelflag_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif /*- #ifdef CLUSTERED_SITE */
