/*
 * $Log: dbinfoAdd.c,v $
 * Revision 1.2  2020-04-01 18:54:02+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-17 12:48:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"
#include "open_master.h"
#include "create_table.h"
#include "variables.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoAdd.c,v 1.2 2020-04-01 18:54:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
static void
die_nomem()
{
	strerr_warn1("dbinfoAdd: out of memory", 0);
	_exit(111);
}

/*
 *
 * File      : controldir/mcdinfo
 * domain    : indimail.org
 * distFlag  : 0
 * server    : 210.210.122.80
 * mdahost   : 210.210.122.80
 * port      : 3306
 * database  : indimail
 * user      : indimail
 * password  : ssh-1.5-
 * timestamp : 20021130003200
 */
int
dbinfoAdd(char *domain, int distributed, char *sqlserver, char *mdahost, int port, int use_ssl, char *database, char *user, char *passwd)
{
	static stralloc SqlBuf = {0}, mcdFile = {0};
	char           *mcdfile, *sysconfdir, *controldir;
	int             err;
	char            strnum[FMT_ULONG];

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/') {
		if (!stralloc_copys(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
			die_nomem();
	} else {
		if (*controldir == '/') {
			if (!stralloc_copys(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
				die_nomem();
		} else {
			if (!stralloc_copys(&mcdFile, sysconfdir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
				die_nomem();
		}
	}
	mcdFile.len--;
	if (open_master()) {
		strerr_warn1("dbinfoAdd: Failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into dbinfo ", 32) ||
			!stralloc_catb(&SqlBuf, "(filename, domain, distributed, server, mdahost, port, use_ssl, dbname, user, passwd) values ", 93) ||
			!stralloc_catb(&SqlBuf, "(\"", 2) ||
			!stralloc_cat(&SqlBuf, &mcdFile) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, domain) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) distributed)) ||
			!stralloc_catb(&SqlBuf, ", \"", 3) ||
			!stralloc_cats(&SqlBuf, sqlserver) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, mdahost) ||
			!stralloc_catb(&SqlBuf, "\", ", 3) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) port)) ||
			!stralloc_catb(&SqlBuf, ", ", 2) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) use_ssl)) ||
			!stralloc_catb(&SqlBuf, ", \"", 3) ||
			!stralloc_cats(&SqlBuf, database) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, passwd) ||
			!stralloc_catb(&SqlBuf, "\")", 2) || !stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("dbinfoAdd: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
		} else {
			strerr_warn4("dbinfoAdd: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("dbinfoAdd: mysql_query: msyql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (!verbose)
		return (!err);
	if (err) {
		out("dbinfoAdd", "Added domain ");
		out("dbinfoAdd",  domain);
		out("dbinfoAdd", "\n");
		flush("dbinfoAdd");
	} else {
		out("dbinfoAdd", "No domain added ");
		out("dbinfoAdd",  domain);
		out("dbinfoAdd", "\n");
		flush("dbinfoAdd");
	}
	return (!err);
}
#endif
