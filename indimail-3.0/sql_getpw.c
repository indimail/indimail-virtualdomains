/*
 * $Log: sql_getpw.c,v $
 * Revision 1.1  2019-04-18 15:49:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef HAVE_PWD_H
#include <pwd.h>
#endif
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <scan.h>
#include <strerr.h>
#include <env.h>
#endif
#include "iopen.h"
#include "get_real_domain.h"
#include "variables.h"
#include "lowerit.h"
#include "munch_domain.h"
#include "vlimits.h"
#include "strToPw.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_getpw.c,v 1.1 2019-04-18 15:49:41+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

static void
die_nomem()
{
	strerr_warn1("sql_getpw: out of memory", 0);
	_exit(111);
}

struct passwd  *
sql_getpw(char *user, char *domain)
{
	char           *domstr, *pwstruct, *real_domain, *ptr;
	int             row_count, pass, err;
	static struct passwd pwent;
	static stralloc IPass = {0}, IGecos = {0}, IDir = {0}, IShell = {0};
	static stralloc SqlBuf = {0}, _user = {0}, _domain = {0};
	static stralloc IUser = {0}, IDomain = {0};
	MYSQL_RES      *res;
	MYSQL_ROW       row;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

	if (!domain || !*domain || !user || !*user)
		return ((struct passwd *) 0);
	if (!stralloc_copys(&_user, user) || !stralloc_0(&_user))
		die_nomem();
	_user.len--;
	if (!stralloc_copys(&_domain, domain) || !stralloc_0(&_domain))
		die_nomem();
	_domain.len--;
	lowerit(_user.s);
	lowerit(_domain.s);
	if (!pwent.pw_name) {/*- first time */
		pwent.pw_name = _user.s;
		pwent.pw_passwd = IPass.s;
		pwent.pw_gecos = IGecos.s;
		pwent.pw_dir = IDir.s;
		pwent.pw_shell = IShell.s;
	}
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("PASSWD_CACHE")) {
		if (IUser.len && IDomain.len && !str_diffn(IUser.s, _user.s, _user.len + 1) &&
				!str_diffn(IDomain.s, _domain.s, _domain.len + 1))
			return (&pwent);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	is_inactive = userNotFound = is_overquota = 0;
	if ((pwstruct = env_get("PWSTRUCT"))) {
		for (ptr = pwstruct;*ptr && *ptr != '@';ptr++);
		if (*ptr == '@') {
			*ptr = 0;
			if (!str_diffn(_user.s, pwstruct, _user.len) && !str_diffn(_domain.s, ptr + 1, _domain.len)) {
				*ptr = '@';
				return (strToPw(pwstruct, str_len(pwstruct) + 1));
			}
			*ptr = '@';
		} else
			return (strToPw(pwstruct, str_len(pwstruct) + 1));
	}
	if (iopen((char *) 0))
		return ((struct passwd *) 0);
	if (!(real_domain = get_real_domain(_domain.s)))
		real_domain = _domain.s;
	if (site_size == LARGE_SITE) {
		domstr = (char *) 0;
		if (!real_domain || !*real_domain)
			domstr = MYSQL_LARGE_USERS_TABLE;
		else
		if (domain && *domain)
			domstr = munch_domain(real_domain);
		if (!stralloc_copyb(&SqlBuf, "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, ", 57) ||
			!stralloc_catb(&SqlBuf, "pw_gecos, pw_dir, pw_shell from ", 32) ||
			!stralloc_cats(&SqlBuf, domstr) ||
			!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
			!stralloc_cat(&SqlBuf, &_user) ||
			!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, ", 57) ||
			!stralloc_catb(&SqlBuf, "pw_gecos, pw_dir, pw_shell from ", 32) ||
			!stralloc_cats(&SqlBuf, default_table) ||
			!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
			!stralloc_cat(&SqlBuf, &_user) ||
			!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
			!stralloc_cats(&SqlBuf, real_domain) ||
			!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
			die_nomem();
	}
	for (pass = 1;pass <= 2;pass++) {
		if (pass == 2) {
			if (!stralloc_copyb(&SqlBuf, "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, ", 57) ||
				!stralloc_catb(&SqlBuf, "pw_gecos, pw_dir, pw_shell from ", 32) ||
				!stralloc_cats(&SqlBuf, inactive_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name = \"", 18) ||
				!stralloc_cat(&SqlBuf, &_user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
				die_nomem();
		}
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			err = in_mysql_errno(&mysql[1]);
			if (err == ER_NO_SUCH_TABLE || err == ER_SYNTAX_ERROR)
				userNotFound = 1;
			else
				strerr_warn4("sql_getpw: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		if (!(res = in_mysql_store_result(&mysql[1]))) {
			strerr_warn2("sql_getpw: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
			return ((struct passwd *) 0);
		}
		if ((row_count = in_mysql_num_rows(res)))
			break;
		else
			in_mysql_free_result(res);
	}
	if (!row_count) {
		userNotFound = 1;
		return ((struct passwd *) 0);
	}
	if (pass == 2)
		is_inactive = 1;
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(&_user, row[0]) || !stralloc_0(&_user))
			die_nomem();
		_user.len--;
		if (!stralloc_copys(&IPass, row[1]) || !stralloc_0(&IPass))
			die_nomem();
		IPass.len--;
		scan_uint(row[2], &pwent.pw_uid);
		scan_uint(row[3], &pwent.pw_gid);
		if (pwent.pw_gid & BOUNCE_MAIL)
			is_overquota = 1;
		if (!stralloc_copys(&IGecos, row[4]) || !stralloc_0(&IGecos))
			die_nomem();
		IGecos.len--;
		if (!stralloc_copys(&IDir, row[5]) || !stralloc_0(&IDir))
			die_nomem();
		IDir.len--;
		if (!stralloc_copys(&IShell, row[6]) || !stralloc_0(&IShell))
			die_nomem();
		IShell.len--;
		if (!stralloc_copy(&IUser, &_user) || !stralloc_0(&IUser))
			die_nomem();
		IUser.len--;
		if (!stralloc_copy(&IDomain, &_domain) || !stralloc_0(&IDomain))
			die_nomem();
		IDomain.len--;
		in_mysql_free_result(res);
		pwent.pw_name = _user.s;
		pwent.pw_passwd = IPass.s;
		pwent.pw_gecos = IGecos.s;
		pwent.pw_dir = IDir.s;
		pwent.pw_shell = IShell.s;
#ifdef ENABLE_DOMAIN_LIMITS
		if (env_get("DOMAIN_LIMITS") && !(pwent.pw_gid & V_OVERRIDE)) {
			if (!vget_limits(_domain.s, &limits))
				pwent.pw_gid |= vlimits_get_flag_mask(&limits);
			else
				return ((struct passwd *) 0);
		}
#endif
		return (&pwent);
	} 
	in_mysql_free_result(res);
	return ((struct passwd *) 0);
}

#ifdef QUERY_CACHE
void
sql_getpw_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
