/*
 * $Log: vpriv_update.c,v $
 * Revision 1.1  2019-04-15 12:29:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_update.c,v 1.1 2019-04-15 12:29:40+05:30 Cprogrammer Exp mbhangui $";
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
#include <strmsg.h>
#include <fmt.h>
#endif
#include "findhost.h"
#include "create_table.h"
#include "variables.h"
#include "open_master.h"

static void
die_nomem()
{
	strerr_warn1("vpriv_update: out of memory", 0);
	_exit(111);
}

int
vpriv_update(char *user, char *program, char *cmdargs)
{
	int             err;
	static stralloc SqlBuf = {0};
	char            strnum[FMT_ULONG];

	if (!user || !*user || !program || !*program || !cmdargs || !*cmdargs)
		return (1);
	if (open_master()) {
		strerr_warn1("vpriv_update: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority vpriv set cmdswitches=\"", 43) ||
			!stralloc_cats(&SqlBuf, cmdargs) ||
			!stralloc_catb(&SqlBuf, "\" where user=\"", 14) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\" and program=\"", 15) ||
			!stralloc_cats(&SqlBuf, program) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			strmsg_out5("No program ", program, " for user ", user, "\n");
			if (create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT))
				return (-1);
			return (1);
		}
		strerr_warn4("vpriv_update: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("vpriv_update: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err) {
		strnum[fmt_ulong(strnum, (unsigned long) err)] = 0;
		strmsg_out9("Updated Cmdargs ", cmdargs, " for user ", user, " program ", program, " (", strnum, " entries)\n");
	} else
		strerr_warn4("No Program ", program, " for user ", user, 0);
	return (0);
}
#endif
