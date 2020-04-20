/*
 * $Log: sql_renamedomain.c,v $
 * Revision 1.1  2019-04-15 12:39:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sql_renamedomain.c,v 1.1 2019-04-15 12:39:38+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <error.h>
#include <substdio.h>
#include <getln.h>
#endif
#include "open_master.h"
#include "iopen.h"
#include "variables.h"
#include "munch_domain.h"
#include "is_distributed_domain.h"

static void
die_nomem()
{
	strerr_warn1("sql_renamedomain: out of memory", 0);
	_exit(111);
}

static int
rename_data(int which, char *tablename, char *column_name, char *NewDomain, char *OldDomain)
{
	static stralloc SqlBuf = {0};

	if (which != ON_MASTER && which != ON_LOCAL)
		return (-1);
	if (!stralloc_copyb(&SqlBuf, "update ", 7) ||
			!stralloc_cats(&SqlBuf, tablename) ||
			!stralloc_catb(&SqlBuf, " set ", 5) ||
			!stralloc_cats(&SqlBuf, column_name) ||
			!stralloc_catb(&SqlBuf, "=\"", 2) ||
			!stralloc_cats(&SqlBuf, NewDomain) ||
			!stralloc_catb(&SqlBuf, "\" where ", 8) ||
			!stralloc_cats(&SqlBuf, column_name) ||
			!stralloc_catb(&SqlBuf, "=\"", 2) ||
			!stralloc_cats(&SqlBuf, OldDomain) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
#ifdef CLUSTERED_SITE
	if ((which == ON_MASTER ? open_master() : iopen((char *) 0))) {
		strerr_warn2("rename_data: Failed to open ", which == ON_MASTER ? "master db" : "local db", 0);
		return (-1);
	}
#else
	which = ON_LOCAL;
	if (iopen((char *) 0)) {
		strerr_warn1("rename_data: Failed to open local db", 0);
		return (-1);
	}
#endif
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(which == ON_MASTER ? &mysql[0] : &mysql[1]) == ER_NO_SUCH_TABLE)
			return (0);
		strerr_warn4("rename_data: ", SqlBuf.s, ": ", (char *) in_mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), 0);
		return (-1);
	}
	return (0);
}

int
sql_renamedomain(char *OldDomain, char *NewDomain, char *domdir)
{
	static stralloc SqlBuf = {0}, tmp1 = {0}, tmp2 = {0}, line = {0};
	char           *ptr1, *ptr2;
	int             fd, match, err = 0;
#ifdef CLUSTERED_SITE
	int             is_dist = 0;
#endif
	char            inbuf[4096];
	struct substdio ssin;

	if (iopen((char *) 0))
		return (-1);
	if (site_size == LARGE_SITE) {
		ptr1 = munch_domain(OldDomain);
		ptr2 = munch_domain(NewDomain);
		if (!stralloc_copyb(&SqlBuf, "rename table ", 13) ||
				!stralloc_cats(&SqlBuf, ptr1) ||
				!stralloc_catb(&SqlBuf, " to ", 4) ||
				!stralloc_cats(&SqlBuf, ptr2) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			strerr_warn4("sql_renamedomain: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	} else
	if (rename_data(ON_LOCAL, default_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(OldDomain)) == -1) {
		strerr_warn2(OldDomain, ": is_distributed_domain failed", 0);
		return (-1);
	}
	if (is_dist && rename_data(ON_MASTER, cntrl_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_MASTER, "smtp_port", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_MASTER, "dbinfo", "domain", NewDomain, OldDomain))
		err = 1;
	else
#endif
	if (rename_data(ON_LOCAL, inactive_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "valias", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "dir_control", "domain", NewDomain, OldDomain))
		err = 1;
	if (!stralloc_copys(&tmp1, domdir) ||
			!stralloc_catb(&tmp1, "/.filesystems", 13) ||
			!stralloc_0(&tmp1))
		die_nomem();
	if ((fd = open_read(tmp1.s)) == -1) {
		if (errno != error_noent)
			strerr_warn3("vrenamedomain: open: ", tmp1.s, ": ", &strerr_sys);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vrenamedomain: read: ", tmp1.s, ": ", &strerr_sys);
			close(fd);
			return (1);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		if (!stralloc_copyb(&tmp2, "dir_control", 11) ||
				!stralloc_cat(&tmp2, &line) ||
				!stralloc_0(&tmp2))
			die_nomem();
		if (rename_data(ON_LOCAL, tmp2.s, "domain", NewDomain, OldDomain))
			err = 1;
	}
	close(fd);
	if (rename_data(ON_LOCAL, "lastauth", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "userquota", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "vlimit", "domain", NewDomain, OldDomain))
		err = 1;
	return (err);
}
