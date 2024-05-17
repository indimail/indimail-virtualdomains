/*
 * $Log: valias_delete_domain.c,v $
 * Revision 1.2  2022-10-27 17:29:20+05:30  Cprogrammer
 * delete entries for alias instead of real domain
 *
 * Revision 1.1  2019-04-15 11:59:49+05:30  Cprogrammer
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
#endif
#include "indimail.h"
#include "get_real_domain.h"
#include "create_table.h"
#include "iopen.h"

#ifndef	lint
static char     sccsid[] = "$Id: valias_delete_domain.c,v 1.2 2022-10-27 17:29:20+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#include <mysql.h>
#include <mysqld_error.h>
static void
die_nomem()
{
	strerr_warn1("valias_delete_domain: out of memory", 0);
	_exit(111);
}

int
valias_delete_domain(const char *domain)
{
	int             err;
	static stralloc SqlBuf = {0};

	if (!domain || !*domain)
		return (1);
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from valias where domain = \"", 48) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			return (0);
		}
		strerr_warn4("valias_delete_domain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	return (0);
}
#endif
