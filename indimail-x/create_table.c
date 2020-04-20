/*
 * $Log: create_table.c,v $
 * Revision 1.1  2019-04-14 18:34:50+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#endif
#include "indimail.h"
#include "open_master.h"
#include "layout.h"
#include "iopen.h"

#ifndef	lint
static char     sccsid[] = "$Id: create_table.c,v 1.1 2019-04-14 18:34:50+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("create_table: out of memory", 0);
	_exit(111);
}

int
create_table(int which, char *table, char *_template)
{
	char           *SqlBuf, *template;
	int             len;

	if (!table || !*table)
		return (-1);
	if (!(template = (!_template || !*_template) ? layout(table) : _template)) {
		strerr_warn2("create_table: Invalid template for table ", table, 0);
		return (-1);
	}
	if (which != ON_MASTER && which != ON_LOCAL)
		return (-1);
#ifdef CLUSTERED_SITE
	if ((which == ON_MASTER ? open_master() : iopen((char *) 0))) {
		strerr_warn2("create_table: failed to open ", which == ON_MASTER ? "master db" : "local db", 0);
		return (-1);
	}
#else
	if (which == ON_MASTER)
		return (0);
	if (iopen((char *) 0)) {
		strerr_warn2("create_table: failed to open ", which == ON_MASTER ? "central db" : "local db", 0);
		return (-1);
	}
#endif
	len = str_len(table) + str_len(template) + 33;
	if (!(SqlBuf = (char *) alloc(len)))
		die_nomem();
	len = fmt_strn(SqlBuf, "create table IF NOT EXISTS ", 27);
	len += fmt_str(SqlBuf + len, table);
	len += fmt_strn(SqlBuf + len, " ( ", 3);
	len += fmt_str(SqlBuf + len, template);
	len += fmt_strn(SqlBuf + len, " )", 2);
	SqlBuf[len] = 0;
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf)) {
		strerr_warn6("create_table: ", table, "\nQuery: ", SqlBuf, ": ",
			(char *) in_mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), 0);
		alloc_free(SqlBuf);
		return (-1);
	}
	alloc_free(SqlBuf);
	return (0);
}
