/*
 * $Log: dbinfoDel.c,v $
 * Revision 1.1  2019-04-12 20:42:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoDel.c,v 1.1 2019-04-12 20:42:09+05:30 Cprogrammer Exp mbhangui $";
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
#include "getEnvConfig.h"
#include "create_table.h"
#include "open_master.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("dbinfoDel: out of memory", 0);
	_exit(111);
}

int
dbinfoDel(char *domain, char *mdahost)
{
	static stralloc SqlBuf = {0}, mcdFile = {0};
	char           *mcdfile, *sysconfdir, *controldir;
	int             err;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(*mcdfile == '/') {
		if (!stralloc_copys(&mcdFile, mcdfile) ||
				!stralloc_0(&mcdFile))
			die_nomem();
	}else {
		if (*controldir == '/') {
			if (!stralloc_copys(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) ||
					!stralloc_0(&mcdFile))
				die_nomem();
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (!stralloc_copys(&mcdFile, sysconfdir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) ||
					!stralloc_0(&mcdFile))
				die_nomem();
		}
	}
	mcdFile.len--;
	if (open_master()) {
		strerr_warn1("dbinfoDel: Failed to open master db", 0);
		return(-1);
	}
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from dbinfo where filename=\"", 48) ||
			!stralloc_cat(&SqlBuf, &mcdFile) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\" and mdahost=\"", 15) ||
			!stralloc_cats(&SqlBuf, mdahost) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT))
				return(-1);
			return(0);
		} else {
			strerr_warn4("dbinfoDel: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
	}
	if((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("dbinfoDel: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return(-1);
	}
	if (verbose) {
		out("dbinfoDel", err ? "Deleted Domain " : "No MCD for domain ");
		out("dbinfoDel", domain);
		out("dbinfoDel", "\n");
		flush("dbinfoDel");
	}
	return (0);
}
#endif
