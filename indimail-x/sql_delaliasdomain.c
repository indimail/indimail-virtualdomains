/*
 * $Log: sql_delaliasdomain.c,v $
 * Revision 1.1  2019-04-14 22:51:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CLUSTERED_SITE
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#endif
#include "indimail.h"
#include "open_master.h"
#include "create_table.h"
#include "findhost.h"
#include "islocalif.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_delaliasdomain.c,v 1.1 2019-04-14 22:51:53+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("sql_delaliasdomain: out of memory", 0);
	_exit(111);
}

int
sql_delaliasdomain(const char *aliasdomain)
{
	char           *mailstore;
	int             i;
	static stralloc tmpbuf = {0}, SqlBuf = {0};

	if (open_master()) {
		strerr_warn1("sql_delaliasdomain: failed to open master db", 0);
		return (1);
	}
	if (!stralloc_copyb(&tmpbuf, "postmaster@", 11) ||
			!stralloc_cats(&tmpbuf, aliasdomain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	/*- domain:[ip|host]:port */
	if ((mailstore = findhost(tmpbuf.s, 1)) != (char *) 0) {
		i = str_rchr(mailstore, ':');
		if (mailstore[i])
			mailstore[i] = 0;
		else {
			strerr_warn2("sql_delaliasdomain: invalid smtproute", mailstore, 0);
			return (1);
		}
		for (; *mailstore && *mailstore != ':'; mailstore++);
		if (!*mailstore || !*(mailstore + 1)) {
			strerr_warn2("sql_delaliasdomain: invalid smtproute", mailstore, 0);
			return (1);
		}
		mailstore++;
		if (!islocalif(mailstore)) {
			strerr_warn5("sql_delaliasdomain: postmaster@", aliasdomain, \
				" not local (mailstore ", mailstore, "). Not deleting alias domain", 0);
			return (1);
		}
	} else {
		strerr_warn1("sql_delaliasdomain: can't figure out postmaster host", 0);
		return (1);
	}
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from aliasdomain where alias=\"", 50) ||
			!stralloc_cats(&SqlBuf, aliasdomain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if(in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if(create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT))
				return(-1);
			return(0);
		}
		strerr_warn4("sql_delaliasdomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	}
	return (0);
}
#endif
