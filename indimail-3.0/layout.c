/*
 * $Log: layout.c,v $
 * Revision 1.1  2019-04-18 08:25:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <strerr.h>
#endif
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: layout.c,v 1.1 2019-04-18 08:25:41+05:30 Cprogrammer Exp mbhangui $";
#endif

char           *
layout(char *tablename)
{
	struct layout
	{
		char           *tablename;
		char           *layout;
	};
	static struct layout ptr[] = {
		{"dbinfo", DBINFO_TABLE_LAYOUT},
#ifdef CLUSTERED_SITE
		{"hostcntrl", CNTRL_TABLE_LAYOUT},
		{"host_table", HOST_TABLE_LAYOUT},
		{"smtp_port", SMTP_TABLE_LAYOUT},
		{"aliasdomain", ALIASDOMAIN_TABLE_LAYOUT},
		{"spam", SPAM_TABLE_LAYOUT},
		{"badmailfrom", BADMAILFROM_TABLE_LAYOUT},
		{"badrcptto", BADMAILFROM_TABLE_LAYOUT},
#endif
		{"indimail", SMALL_TABLE_LAYOUT},
		{"indibak", SMALL_TABLE_LAYOUT},
		{"indimail", LARGE_TABLE_LAYOUT},
#if defined(POP_AUTH_OPEN_RELAY)
		{"relay", RELAY_TABLE_LAYOUT},
#endif
#ifdef IP_ALIAS_DOMAINS
		{"ip_alias_map", IP_ALIAS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_AUTH_LOGGING
		{"lastauth", LASTAUTH_TABLE_LAYOUT},
		{"userquota", USERQUOTA_TABLE_LAYOUT},
#endif
		{"dir_control_", DIR_CONTROL_TABLE_LAYOUT},
#ifdef VALIAS
		{"valias", VALIAS_TABLE_LAYOUT},
#endif
		{"mgmtaccess", MGMT_TABLE_LAYOUT},
#ifdef ENABLE_MYSQL_LOGGING
		{"vlog", VLOG_TABLE_LAYOUT},
#endif
		{"bulkmail", BULKMAIL_TABLE_LAYOUT},
		{"fstab", FSTAB_TABLE_LAYOUT},
#ifdef VFILTER
		{"vfilter", FILTER_TABLE_LAYOUT},
#endif
#ifdef ENABLE_DOMAIN_LIMITS
		{"vlimits", LIMITS_TABLE_LAYOUT},
#endif
		{0, 0}
	};
	int             i;

	for (i = 0; ptr[i].tablename; i++)
	{
		if (!str_diffn(ptr[i].tablename, tablename, str_len(ptr[i].tablename) + 1))
			return (ptr[i].layout);
	}
	strerr_warn2("layout: No layout for ", tablename, 0);
	return ((char *) 0);
}
