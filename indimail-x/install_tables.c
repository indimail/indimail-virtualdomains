/*
 * $Log: install_tables.c,v $
 * Revision 1.2  2020-04-01 18:55:57+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:20:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <getEnvConfig.h>
#include <subfd.h>
#endif
#include "create_table.h"
#include "open_master.h"
#include "common.h"
#include "variables.h"
#include "indimail.h"
#include "disable_mysql_escape.h"

#ifndef	lint
static char     sccsid[] = "$Id: install_tables.c,v 1.2 2020-04-01 18:55:57+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("install_tables: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
#ifdef POP_AUTH_OPEN_RELAY
	char           *relay_table;
#endif
#ifdef CLUSTERED_SITE
	static stralloc host_path = {0};
	char           *sysconfdir, *controldir;
	int             hostmaster_present = 0;
#endif
	int             i;

	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	disable_mysql_escape(1);
	for (i = 0; i < 2; i++) {
		if (create_table(ON_LOCAL, i == 0 ? default_table : inactive_table,
				 site_size == LARGE_SITE ? LARGE_TABLE_LAYOUT : SMALL_TABLE_LAYOUT))
		{
			strerr_warn2("install_tables: failed to create table ", i == 0 ? default_table : inactive_table, 0);
			return (1);
		} else {
			subprintfe(subfdout, "install_tables", "created table %s on local\n",
					i == 0 ? default_table : inactive_table);
			flush("install_tables");
		}
	}
#if defined(POP_AUTH_OPEN_RELAY)
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	if (create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
		return (1);
	else {
		subprintfe(subfdout, "install_tables", "created table %s on local\n", relay_table);
		flush("install_tables");
	}
#endif
#ifdef CLUSTERED_SITE
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.master", 12) ||
				!stralloc_0(&host_path))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&host_path, sysconfdir) ||
				!stralloc_append(&host_path, "/") ||
				!stralloc_cats(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.master", 12) ||
				!stralloc_0(&host_path))
			die_nomem();
	}
	if (!access(host_path.s, F_OK) || env_get("MASTER_HOST"))
		hostmaster_present = open_master() ? 0 : 1;
	for (i = 0; IndiMailTable[i].table_name; i++) {
		if (IndiMailTable[i].which == ON_MASTER && !hostmaster_present) {
			subprintfe(subfdout, "install_tables", "skipped table %s on %s\n",
					IndiMailTable[i].table_name,
					IndiMailTable[i].which == ON_LOCAL ? "local" : "master");
			flush("install_tables");
			continue;
		}
		if (create_table(IndiMailTable[i].which, IndiMailTable[i].table_name, IndiMailTable[i].template)) {
			strerr_warn2("install_tables: failed to create table ", IndiMailTable[i].table_name, 0);
			return (1);
		} else {
			subprintfe(subfdout, "install_tables", "created table %s on %s",
					IndiMailTable[i].table_name,
					IndiMailTable[i].which == ON_LOCAL ? "local" : "master");
			flush("install_tables");
		}
	}
	getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
	if (!hostmaster_present)
		return (0);
	if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
		return (1);
	else {
		subprintfe(subfdout, "install_tables", "created table %s on master\n", cntrl_table);
		flush("install_tables");
	}
#endif
	return (0);
}
