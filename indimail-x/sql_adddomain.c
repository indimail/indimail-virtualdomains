/*
 * $Log: sql_adddomain.c,v $
 * Revision 1.2  2021-02-23 21:40:48+05:30  Cprogrammer
 * replaced CREATE TABLE statements with create_table() function
 *
 * Revision 1.1  2019-04-20 08:34:29+05:30  Cprogrammer
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
#include "iopen.h"
#include "variables.h"
#include "munch_domain.h"
#include "create_table.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_adddomain.c,v 1.2 2021-02-23 21:40:48+05:30 Cprogrammer Exp mbhangui $";
#endif

int
sql_adddomain(char *domain)
{
	int             err;

	if ((err = iopen((char *) 0)))
		return (err);
	if (create_table(ON_LOCAL, site_size == SMALL_SITE ? default_table : (domain && *domain ? munch_domain(domain) : MYSQL_LARGE_USERS_TABLE), site_size == SMALL_SITE ? SMALL_TABLE_LAYOUT : LARGE_TABLE_LAYOUT))
		return 1;
	if (site_size == SMALL_SITE && create_table(ON_LOCAL, inactive_table, SMALL_TABLE_LAYOUT))
		return 1;
	return (0);
}
