/*
 * $Id: $
 */
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <open.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <scan.h>
#include <sgetopt.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <getEnvConfig.h>
#endif
#include "iopen.h"
#include "common.h"
#include "iclose.h"
#include "variables.h"
#include "create_table.h"

#ifndef	lint
static char     rcsid[] = "$Id: ischema.c,v 1.3 2023-03-20 10:08:51+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "ischema: fatal: "
#define WARN    "ischema: warning: "

static char    *usage =
	"usage: ischema [options]\n"
	"         -v          (verbose)\n"
	"         -u          update schema"
	;

static void
die_nomem()
{
	strerr_warn1("out of memory", 0);
	_exit(111);
}

int
update_schema(char *sql_stmt)
{
	int             err;

	if (mysql_query(&mysql[1], sql_stmt)) {
		strerr_warn5(FATAL, "mysql_query [", sql_stmt, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
		return -1;
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn3(WARN, "mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return -1;
	}
	subprintfe(subfdout, "ischema", "%d rows updated\n", err);
	flush("LoadBMF");
	return err;
}

int
main(int argc, char **argv)
{
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             err, ign, id, l_id, fd, c, update_mode = 0, match,
					u_count;
	char            inbuf[4096];
	char           *p1, *p2, *lid_str, *command, *comment, *ignore,
				   *sql_stmt, *sysconfdir;
	struct substdio ssin;
	static stralloc line = {0}, SqlBuf = {0}, tmpbuf = {0};

	while ((c = getopt(argc, argv, "vdu")) != opteof) {
		switch (c)
		{
		case 'v':
			break;
		case 'u':
			update_mode = 1;
			break;
		default:
			strerr_die2x(100, WARN, usage);
		}
	}
	if (iopen((char *) 0) != 0)
		_exit(111);
	if (!stralloc_copyb(&SqlBuf, "select MAX(id) from ischema", 27) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	for (;;) {
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
				if (create_table(ON_LOCAL, "ischema", SCHEMA_TABLE_LAYOUT)) {
					strerr_warn3(FATAL, "failed to create ischema table: ", (char *) in_mysql_error(&mysql[1]), 0);
					_exit(111);
				}
				continue;
			}
			strerr_warn5(FATAL, "mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			_exit(111);
		} else
			break;
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn3(FATAL, "in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		_exit(111);
	}
	if ((row = in_mysql_fetch_row(res))) {
		subprintfe(subfdout, "ischema", "Schema Version = %s\n", row[0] ? row[0] : "0");
		if (row[0])
			scan_int(row[0], &id);
		else
			id = 0;
	} else
	subprintfe(subfderr, "ischema", "no rows fetched\n");
	flush("ischema");
	if (substdio_flush(subfderr) == -1)
		strerr_die1sys(111, "write: unable to write output: ");
	if (!update_mode)
		_exit(0);
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	if (!stralloc_copys(&tmpbuf, sysconfdir) ||
			!stralloc_catb(&tmpbuf, "/indimail.schema", 17) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1)
		strerr_die4sys(111, FATAL, "open: ", tmpbuf.s, ": ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	/* id:sql|cmd:IGNORE Yes|NO:comment:sql_stmt */
	for (u_count = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn4(FATAL, "read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			iclose();
			_exit(111);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (p1 = line.s; *p1 && isspace((int) *p1); p1++);
		if (!*p1)
			continue;
		/*- get id */
		for (p2 = p1 + 1; *p2 && *p2 != ':'; p2++);
		if (*p2 == ':') {
			*p2 = 0;
			scan_int(p1, &l_id);
			lid_str = p1;
			p1 = p2 + 1;
		} else
			continue;
		/*- get command */
		for (p2 = p1 + 1;*p2 && *p2 != ':'; p2++);
		if (*p2 == ':') {
			*p2 = 0;
			command = p1;
			p1 = p2 + 1;
		} else
			continue;
		/*- get ignore */
		for (p2 = p1 + 1;*p2 && *p2 != ':'; p2++);
		if (*p2 == ':') {
			*p2 = 0;
			ignore = p1;
			p1 = p2 + 1;
		} else
			continue;
		/*- get comment */
		for (p2 = p1 + 1;*p2 && *p2 != ':'; p2++);
		if (*p2 == ':') {
			*p2 = 0;
			comment = p1;
			p1 = p2 + 1;
		} else
			continue;
		/*- get sql statement */
		sql_stmt = p1;
		if (l_id <= id)
			continue;
		u_count++;
		subprintfe(subfdout, "ischema", "%3s: command=%-5s ignore=%-3s comment=%-25s sql=%s\n",
					lid_str, command, ignore, comment, sql_stmt);
		flush("ischema");
		ign = str_diff(ignore, "YES") ? 0 : 1;
		if (!str_diff(command, "sql")) {
			if (mysql_query(&mysql[1], sql_stmt)) {
				if (ign)
					continue;
				strerr_warn5(FATAL, "mysql_query [", sql_stmt, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				break;
			}
			if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
				if (ign)
					continue;
				strerr_warn3(WARN, "mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
				break;
			}
			if (!stralloc_copyb(&SqlBuf, "INSERT LOW_PRIORITY INTO ischema (comment) VALUES (\"", 52) ||
					!stralloc_cats(&SqlBuf, comment) ||
					!stralloc_catb(&SqlBuf, "\")", 2) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
			if (update_schema(SqlBuf.s) == -1)
				break;
		}
	} /*- for (;;) */
	close(fd);
	if (!u_count) {
		subprintfe(subfdout, "ischema", "no schema updates available\n");
		flush("ischema");
	}
	return 0;
}
/*
 * $Log: ischema.c,v $
 * Revision 1.3  2023-03-20 10:08:51+05:30  Cprogrammer
 * use SYSCONFDIR env variable for indimail.schema
 * standardize getln handling
 *
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2022-08-05 19:23:26+05:30  Cprogrammer
 * Initial revision
 *
 */
