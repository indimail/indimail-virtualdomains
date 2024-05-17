/*
 * $Log: limits.c,v $
 * Revision 1.5  2023-03-23 22:08:24+05:30  Cprogrammer
 * removed spurious warning message when limits doesn't exist
 *
 * Revision 1.4  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.3  2019-04-22 23:12:57+05:30  Cprogrammer
 * replaced atoi(), atol() with scan_int(), scan_ulong() functions
 *
 * Revision 1.2  2019-04-15 21:57:29+05:30  Cprogrammer
 * added vlimits.h
 *
 * Revision 1.1  2019-04-15 12:26:52+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: limits.c,v 1.5 2023-03-23 22:08:24+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_DOMAIN_LIMITS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <byte.h>
#include <scan.h>
#include <fmt.h>
#include <strerr.h>
#include <stralloc.h>
#include <subfd.h>
#endif
#include "indimail.h"
#include "create_table.h"
#include "iopen.h"
#include "variables.h"
#include "common.h"
#include "vlimits.h"

static void     vdefault_limits(struct vlimits *limits);
static stralloc SqlBuf = {0};

static void
die_nomem(char *str)
{
	strerr_warn2(str, ": out of memory", 0);
	_exit(111);
}

int
vget_limits(const char *domain, struct vlimits *limits)
{
	int             err, perm;
	MYSQL_ROW       row;
	MYSQL_RES      *res;

	/*- initialise a limits struct.  */
	vdefault_limits(limits);
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_copyb(&SqlBuf,
		"SELECT domain_expiry, passwd_expiry, maxpopaccounts, maxaliases, maxforwards, "
		"maxautoresponders, maxmailinglists, diskquota, maxmsgcount, defaultquota, "
		"defaultmaxmsgcount, disable_pop, disable_imap, disable_dialup, "
		"disable_passwordchanging, disable_webmail, disable_relay, disable_smtp, "
		"perm_account, perm_alias, perm_forward, perm_autoresponder, perm_maillist, "
		"perm_quota, perm_defaultquota FROM vlimits WHERE domain = \"", 421) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem("vget_limits");
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn3("vget_limits: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("vget_limits: mysql_store_results: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!in_mysql_num_rows(res)) {
		in_mysql_free_result(res);
		return (0);
	}
	if ((row = in_mysql_fetch_row(res))) {
		scan_long(row[0], &(limits->domain_expiry));
		scan_long(row[1], &(limits->passwd_expiry));
		scan_int(row[2], &(limits->maxpopaccounts));
		scan_int(row[3], &(limits->maxaliases));
		scan_int(row[4], &(limits->maxforwards));
		scan_int(row[5], &(limits->maxautoresponders));
		scan_int(row[6], &(limits->maxmailinglists));
		limits->diskquota = strtoll(row[7], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->diskquota == LLONG_MIN || limits->diskquota == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			in_mysql_free_result(res);
			return (-1);
		}
		limits->maxmsgcount = strtoll(row[8], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->maxmsgcount == LLONG_MIN || limits->maxmsgcount == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			in_mysql_free_result(res);
			return (-1);
		}
		limits->defaultquota = strtoll(row[9], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->defaultquota == LLONG_MIN || limits->defaultquota == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			in_mysql_free_result(res);
			return (-1);
		}
		limits->defaultmaxmsgcount = strtoll(row[10], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->defaultmaxmsgcount == LLONG_MIN || limits->defaultmaxmsgcount == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			in_mysql_free_result(res);
			return (-1);
		}
		scan_int(row[11], (int *) &(limits->disable_pop));
		scan_int(row[12], (int *) &(limits->disable_imap));
		scan_int(row[13], (int *) &(limits->disable_dialup));
		scan_int(row[14], (int *) &(limits->disable_passwordchanging));
		scan_int(row[15], (int *) &(limits->disable_webmail));
		scan_int(row[16], (int *) &(limits->disable_relay));
		scan_int(row[17], (int *) &(limits->disable_smtp));
		scan_int(row[18], (int *) &(limits->perm_account));
		scan_int(row[19], (int *) &(limits->perm_alias));
		scan_int(row[20], (int *) &(limits->perm_forward));
		scan_int(row[21], (int *) &(limits->perm_autoresponder));
		scan_int(row[22], &perm);
		limits->perm_maillist = perm & VLIMIT_DISABLE_ALL;
		perm >>= VLIMIT_DISABLE_BITS;
		limits->perm_maillist_users = perm & VLIMIT_DISABLE_ALL;
		perm >>= VLIMIT_DISABLE_BITS;
		limits->perm_maillist_moderators = perm & VLIMIT_DISABLE_ALL;
		scan_int(row[23], (int *) &(limits->perm_quota));
		scan_int(row[24], (int *) &(limits->perm_defaultquota));
	}
	in_mysql_free_result(res);
	return (0);
}

int
vdel_limits(const char *domain)
{
	int             err;

	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_copyb(&SqlBuf, "DELETE low_priority FROM vlimits WHERE domain = \"", 49) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem("vdel_limits");
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT))
				return (-1);
			return (0);
		}
		strerr_warn3("vdel_limits: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("vdel_limits: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err && verbose) {
		subprintfe(subfdout, "vdel_limits", "Deleted limits (%d entries) for domain %s\n", err, domain);
		flush("vdel_limits");
	} else
	if (verbose) {
		subprintfe(subfdout, "vdel_limits", "No limits for domain %s\n", domain);
		flush("vdel_limits");
	}
	return 0;
}

int
vset_limits(const char *domain, struct vlimits *limits)
{
	int             err;
	char            strnum[FMT_ULONG];

	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_copyb(&SqlBuf,
		"REPLACE INTO vlimits (domain, domain_expiry, passwd_expiry, maxpopaccounts, maxaliases, "
		"maxforwards, maxautoresponders, maxmailinglists, diskquota, maxmsgcount, defaultquota, "
		"defaultmaxmsgcount, disable_pop, disable_imap, disable_dialup, "
		"disable_passwordchanging, disable_webmail, disable_relay, disable_smtp, perm_account, "
		"perm_alias, perm_forward, perm_autoresponder, perm_maillist, perm_quota, "
		"perm_defaultquota) VALUES (\"", 425) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, limits->domain_expiry)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, limits->passwd_expiry)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxpopaccounts)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxaliases)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxforwards)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxautoresponders)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxmailinglists)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->diskquota)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->maxmsgcount)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->defaultquota)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->defaultmaxmsgcount)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_pop)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_imap)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_dialup)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_passwordchanging)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_webmail)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_relay)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->disable_smtp)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_account)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_alias)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_forward)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_autoresponder)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_maillist |
						(limits->perm_maillist_users << VLIMIT_DISABLE_BITS) |
						(limits->perm_maillist_moderators << (VLIMIT_DISABLE_BITS * 2)))) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_quota)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, limits->perm_defaultquota)) ||
			!stralloc_catb(&SqlBuf, ")", 1) ||
			!stralloc_0(&SqlBuf))
		die_nomem("vset_limits");
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn3("vset_limits: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		}
		strerr_warn3("vset_limits: ", SqlBuf.s, (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("vset_limits: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!verbose)
		return (err ? 0 : 1);
	if (err)
		subprintfe(subfdout, "vdel_limits", "Added limits for domain %s\n", domain);
	else
		subprintfe(subfdout, "vdel_limits", "No limits added for domain %s\n", domain);
	flush("vset_limits");
	return (err ? 0 : 1);
}

int
vlimits_get_flag_mask(struct vlimits *limits)
{
	int             mask = 0;

	if (limits->disable_pop != 0)
		mask |= NO_POP;
	if (limits->disable_smtp != 0)
		mask |= NO_SMTP;
	if (limits->disable_imap != 0)
		mask |= NO_IMAP;
	if (limits->disable_relay != 0)
		mask |= NO_RELAY;
	if (limits->disable_webmail != 0)
		mask |= NO_WEBMAIL;
	if (limits->disable_passwordchanging != 0)
		mask |= NO_PASSWD_CHNG;
	if (limits->disable_dialup != 0)
		mask |= NO_POP;
	 return mask;
}

static void
vdefault_limits(struct vlimits *limits)
{
	/*- initialize structure */
	byte_zero((char *) limits, sizeof(*limits));

	limits->domain_expiry = -1;
	limits->passwd_expiry = -1;
	limits->maxpopaccounts = -1;
	limits->maxaliases = -1;
	limits->maxforwards = -1;
	limits->maxautoresponders = -1;
	limits->maxmailinglists = -1;
	limits->diskquota = 0;
	limits->maxmsgcount = 0;
	limits->defaultquota = 0;
	limits->defaultmaxmsgcount = 0;

	limits->disable_pop = 0;
	limits->disable_imap = 0;
	limits->disable_dialup = 0;
	limits->disable_passwordchanging = 0;
	limits->disable_webmail = 0;
	limits->disable_relay = 0;
	limits->disable_smtp = 0;
	limits->perm_account = 0;
	limits->perm_alias = 0;
	limits->perm_forward = 0;
	limits->perm_autoresponder = 0;
	limits->perm_maillist = 0;
	limits->perm_maillist_users = 0;
	limits->perm_maillist_moderators = 0;
	limits->perm_quota = 0;
	limits->perm_defaultquota = 0;
}
#endif /*- #ifdef ENABLE_DOMAIN_LIMITS */
