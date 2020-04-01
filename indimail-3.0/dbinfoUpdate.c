/*
 * $Log: dbinfoUpdate.c,v $
 * Revision 1.3  2020-04-01 18:54:13+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-20 08:10:52+05:30  Cprogrammer
 * added missing error message
 *
 * Revision 1.1  2019-04-18 08:31:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoUpdate.c,v 1.3 2020-04-01 18:54:13+05:30 Cprogrammer Exp mbhangui $";
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
#include <fmt.h>
#include <getEnvConfig.h>
#endif
#include "open_master.h"
#include "variables.h"
#include "create_table.h"
#include "common.h"

static void
die_nomem()
{
	strerr_warn1("dbinfoUpdate: out of memory", 0);
	_exit(111);
}

int
dbinfoUpdate(char *domain, int dist, char *sqlserver, char *mdahost, int port,
		int use_ssl, char *database, char *user, char *passwd)
{
	static stralloc SqlBuf = {0}, mcdFile = {0}, optbuf = {0};
	char            strnum[FMT_ULONG];
	char           *mcdfile, *sysconfdir, *controldir;
	int             err, i;

	if (dist < 0) {
		strerr_warn1("dbinfoUpdate: invalid value for dist flag", 0);
		return (-1);
	}
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(*mcdfile == '/') {
		if (!stralloc_copys(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
			die_nomem();
	} else {
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
		strerr_warn1("dbinfoUpdate: Failed to open master db", 0);
		return(-1);
	}
	optbuf.len = 0;
	if (dist != -1) {
		if (!stralloc_copyb(&optbuf, "distributed=", 12) ||
				!stralloc_catb(&optbuf, strnum, fmt_uint(strnum, dist)) ||
				!stralloc_append(&optbuf, ","))
			die_nomem();
	}
	if(sqlserver && *sqlserver) {
		if (!stralloc_copyb(&optbuf, "server=\"", 8) ||
				!stralloc_cats(&optbuf, sqlserver) ||
				!stralloc_catb(&optbuf, "\",", 2))
			die_nomem();
	}
	if(port != -1) {
		if (!stralloc_copyb(&optbuf, "port=", 5) ||
				!stralloc_catb(&optbuf, strnum, fmt_int(strnum, port)) ||
				!stralloc_append(&optbuf, ","))
			die_nomem();
	}
	if(use_ssl != -1) {
		strnum[i = fmt_uint(strnum, use_ssl)] = 0;
		if (!stralloc_copyb(&optbuf, "use_ssl=", 8) ||
				!stralloc_catb(&optbuf, strnum, fmt_uint(strnum, use_ssl)) ||
				!stralloc_append(&optbuf, ","))
			die_nomem();
	}
	if(database && *database) {
		if (!stralloc_copyb(&optbuf, "dbname=\"", 8) ||
				!stralloc_cats(&optbuf, database) ||
				!stralloc_catb(&optbuf, "\",", 2))
			die_nomem();
	}
	if(user && *user) {
		if (!stralloc_copyb(&optbuf, "user=\"", 6) ||
				!stralloc_cats(&optbuf, database) ||
				!stralloc_catb(&optbuf, "\",", 2))
			die_nomem();
	}
	if(passwd && *passwd) {
		if (!stralloc_copyb(&optbuf, "passwd=\"", 8) ||
				!stralloc_cats(&optbuf, passwd) ||
				!stralloc_catb(&optbuf, "\",", 2))
			die_nomem();
	}
	optbuf.s[optbuf.len - 1] = ' '; /*- replace the last comma with space */
	if(!optbuf.len) {
		strerr_warn1("dbinfoUpdate: Invalid Arguments", 0);
		return(-1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority dbinfo set ", 31) ||
			!stralloc_cat(&SqlBuf, &optbuf) ||
			!stralloc_catb(&SqlBuf, "where filename=\"", 16) ||
			!stralloc_cat(&SqlBuf, &mcdFile) ||
			!stralloc_catb(&SqlBuf, "\" and domain=\"", 14) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\" and mdahost=\"", 15) ||
			!stralloc_cats(&SqlBuf, mdahost) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("dbinfoUpdate: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT);
		return (-1);
	}
	if((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("dbinfoUpdate: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return(-1);
	}
	if (!verbose)
		return (!err);
	if(err)
		out("dbinfoUpdate", "Updated Table ");
	else
		out("dbinfoUpdate", "No Update for Table filename[");
	out("dbinfoUpdate", mcdFile.s);
	out("dbinfoUpdate", "], domain ");
	out("dbinfoUpdate", domain);
	out("dbinfoUpdate", ", mdahost ");
	out("dbinfoUpdate", mdahost);
	out("dbinfoUpdate", "\n");
	flush("dbinfoUpdate");
	return (!err);
}
#endif
