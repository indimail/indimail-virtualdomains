/*
 * $Log: valias_update.c,v $
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-15 12:03:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: valias_update.c,v 1.2 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
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
#include "iopen.h"
#include "create_table.h"
#include "variables.h"
#include "common.h"
#include "get_real_domain.h"

static void
die_nomem()
{
	strerr_warn1("valias_update: out of memory", 0);
	_exit(111);
}

int
valias_update(char *alias, char *domain, char *old_alias_line, char *alias_line)
{
	int             err;
	char           *real_domain;
	static stralloc SqlBuf = {0};

	if (!domain || !*domain)
		return (1);
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!(real_domain = get_real_domain(domain)))
		real_domain = domain;
	while (*alias_line == ' ' && *alias_line != 0)
		++alias_line;
	if (!stralloc_copyb(&SqlBuf, "update low_priority valias set valias_line=\"", 44) ||
			!stralloc_cats(&SqlBuf, alias_line) ||
			!stralloc_catb(&SqlBuf, "\" where alias=\"", 15) ||
			!stralloc_cats(&SqlBuf, alias) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_cats(&SqlBuf, real_domain) ||
			!stralloc_catb(&SqlBuf, "\" and valias_line=\"", 19) ||
			!stralloc_cats(&SqlBuf, old_alias_line) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			strerr_warn6("valias_update: No alias line ", alias_line, " for alias ", alias, "@", real_domain, 0);
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			return (1);
		}
		strerr_warn4("valias_update: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("valias_update: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err) {
		subprintfe(subfdout, "valias_update", "Updated alias line %s for alias %s@%s (%d entries)", alias_line, alias, real_domain, err);
		flush("valias_update");
	} else
		strerr_warn6("No alias line ", alias_line, " for alias ", alias, "@", real_domain, 0);
	return (0);
}
#endif
