/*
 * $Log: valias_delete.c,v $
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-15 12:03:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: valias_delete.c,v 1.2 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#endif
#include "get_real_domain.h"
#include "is_distributed_domain.h"
#include "iopen.h"
#include "open_master.h"
#include "create_table.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("valias_delete: out of memory", 0);
	_exit(111);
}

int
valias_delete(char *alias, char *domain, char *alias_line)
{
	int             err;
	static stralloc SqlBuf = {0};
	char           *real_domain;

	if (!domain || !*domain)
		return (1);
	if (!(real_domain = get_real_domain(domain)))
		real_domain = domain;
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1) {
		strerr_warn3("valias_delete: ", real_domain, ": unable to verify if domain is distributed", 0);
		return (1);
	} else
	if (err) {
		if (open_master()) {
			strerr_warn1("valias_delete: failed to open master db", 0);
			return (1);
		}
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from ", 25) ||
				!stralloc_cats(&SqlBuf, cntrl_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
				!stralloc_cats(&SqlBuf, alias) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_catb(&SqlBuf, "\" and pw_passwd=\"alias\"", 23) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
				create_table(ON_MASTER, "hostcntrl", CNTRL_TABLE_LAYOUT);
			else {
				strerr_warn4("valias_delete: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
				return (1);
			}
		}
	}
#endif
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (alias_line && *alias_line) {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from valias where alias = \"", 47) ||
				!stralloc_cats(&SqlBuf, alias) ||
				!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_catb(&SqlBuf, "\" and valias_line=\"", 19) ||
				!stralloc_cats(&SqlBuf, alias_line) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "delete low_priority from valias where alias = \"", 47) ||
				!stralloc_cats(&SqlBuf, alias) ||
				!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			if (verbose) {
				subprintfe(subfdout, "valias_delete", "No alias line %s for alias %s@%s\n",
						alias_line ? alias_line : "", alias, real_domain);
				flush("valias_delete");
			}
			return (0);
		} else {
			strerr_warn4("valias_delete: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("valias_delete: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err && verbose) {
		subprintfe(subfdout, "valias_delete", "Deleted alias line %s for alias %s@%s (%d) entries\n",
				alias_line, alias, real_domain, err);
		flush("valias_delete");
	} else
	if (verbose) {
		subprintfe(subfdout, "valias_delete", "No alias line %s for alias %s@%s\n",
				alias_line ? alias_line : " ", alias, real_domain);
		flush("valias_delete");
	}
	return (0);
}
#endif /*- #ifdef VALIAS */
