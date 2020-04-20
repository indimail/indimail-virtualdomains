/*
 * $Log: vpriv_select.c,v $
 * Revision 1.1  2019-04-15 12:30:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_select.c,v 1.1 2019-04-15 12:30:42+05:30 Cprogrammer Exp mbhangui $";
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
#endif
#include "findhost.h"
#include "create_table.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vpriv_select: out of memory", 0);
	_exit(111);
}

char           *
vpriv_select(char **user, char **program)
{
	static stralloc SqlBuf = {0};
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!select_res) {
		if (open_central_db(0))
			return ((char *) 0);
		if (program && *program) {
			if (user && *user && **user) {
				if (!stralloc_copyb(&SqlBuf, "select high_priority user,program,cmdswitches from vpriv where user=\"", 69) ||
						!stralloc_cats(&SqlBuf, *user) ||
						!stralloc_catb(&SqlBuf, "\" and program=\"", 15) ||
						!stralloc_cats(&SqlBuf, *program) ||
						!stralloc_append(&SqlBuf, "\"") ||
						!stralloc_0(&SqlBuf))
					die_nomem();
			} else {
				if (!stralloc_copyb(&SqlBuf, "select high_priority user,program,cmdswitches from vpriv where program=\"", 72) ||
						!stralloc_cats(&SqlBuf, *program) ||
						!stralloc_append(&SqlBuf, "\"") ||
						!stralloc_0(&SqlBuf))
					die_nomem();
			}
		} else {
			if (user && *user && **user) {
				if (!stralloc_copyb(&SqlBuf, "select high_priority user,program,cmdswitches from vpriv where user=\"", 69) ||
						!stralloc_cats(&SqlBuf, *user) ||
						!stralloc_append(&SqlBuf, "\"") ||
						!stralloc_0(&SqlBuf))
					die_nomem();
			} else {
				if (!stralloc_copyb(&SqlBuf, "select high_priority user,program,cmdswitches from vpriv", 56) ||
						!stralloc_0(&SqlBuf))
					die_nomem();
			}
		}
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
				create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT);
				return ((char *) 0);
			}
			strerr_warn4("vpriv_select: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return ((char *) 0);
		}
		if (!(select_res = in_mysql_store_result(&mysql[0])))
			return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(select_res)))
	{
		*user = row[0];
		*program = row[1];
		return (row[2]);
	}
	in_mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif
