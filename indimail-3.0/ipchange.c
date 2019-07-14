/*
 * $Log: ipchange.c,v $
 * Revision 1.3  2019-06-07 16:06:32+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:12:47+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-15 11:10:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: ipchange.c,v 1.3 2019-06-07 16:06:32+05:30 Cprogrammer Exp mbhangui $";
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
#include <sgetopt.h>
#endif
#include "variables.h"
#include "open_master.h"
#include "iopen.h"
#include "strmsg.h"

#define FATAL   "ipchange: fatal: "
#define WARN    "ipchange: warning: "

static char    *usage =
	"usage: ipchange [options] table_name\n"
	"         -v           ( verbose )\n"
	"         -o old_ip    ( old IP Address )\n"
	"         -n new_ip    ( new IP Address )\n"
	"         -c col_name  ( Column Name )\n"
	"         -m           (table is on hostcntrl)"
	;

int
get_options(int argc, char **argv, char **old_ip, char **new_ip,
	char **table_name, char **column_name, int *which)
{
	int             c;

	*column_name = *old_ip = *new_ip = *table_name = 0;
	*which = ON_LOCAL;
	verbose = 0;
	while ((c = getopt(argc, argv, "vmc:o:n:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'm':
			*which = ON_MASTER;
			break;
		case 'c':
			*column_name = optarg;
			break;
		case 'o':
			*old_ip = optarg;
			break;
		case 'n':
			*new_ip = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*table_name = argv[optind++];
	if (!*old_ip || !*new_ip || !*table_name || !*column_name) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("ipchange: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *old_ip, *new_ip, *table_name, *column_name;
	char            strnum[FMT_ULONG];
	static stralloc SqlBuf = {0};
	int             which, err;

	if (get_options(argc, argv, &old_ip, &new_ip, &table_name, &column_name, &which))
		return (1);
	if (!stralloc_copyb(&SqlBuf, "update low_priority ", 20) ||
			!stralloc_cats(&SqlBuf, table_name) ||
			!stralloc_catb(&SqlBuf, " set ", 5) ||
			!stralloc_cats(&SqlBuf, column_name) ||
			!stralloc_catb(&SqlBuf, "=\"", 2) ||
			!stralloc_cats(&SqlBuf, new_ip) ||
			!stralloc_catb(&SqlBuf, "\" where ", 8) ||
			!stralloc_cats(&SqlBuf, column_name) ||
			!stralloc_catb(&SqlBuf, "=\"", 2) ||
			!stralloc_cats(&SqlBuf, old_ip) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if ((which == ON_MASTER ? open_master() : iopen((char *) 0))) {
		strerr_warn2("ipchange: failed to open ", which == ON_MASTER ? "master db" : "local db", 0);
		return (-1);
	}
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf.s)) {
		strerr_warn4("ipchange: mysql_query [", SqlBuf.s, "]: ",
				(char *) in_mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), 0);
		return (-1);
	}
	if ((err = in_mysql_affected_rows(which == ON_MASTER ? &mysql[0] : &mysql[1])) == -1) {
		strerr_warn2("ipchange: in_mysql_store_results: ",
				(char *) in_mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), 0);
		return (-1);
	}
	if (verbose) {
		strnum[fmt_int(strnum, err)] = 0;
		strmsg_out2(strnum, " rows affected\n");
	}
	return (err ? 0 : 1);
}
#else
#include <sterrr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (0);
}
#endif
