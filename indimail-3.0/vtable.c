/*
 * $Log: vtable.c,v $
 * Revision 1.2  2019-05-28 17:42:54+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-14 21:13:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define _GNU_SOURCE
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#include <str.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#endif
#include "variables.h"
#include "mysql_stack.h"
#include "common.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: vtable.c,v 1.2 2019-05-28 17:42:54+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vadduser: fatal: "
#define WARN    "vadduser: warning: "

int             verbose;
static char    *usage =
	"usage: vtable [options] vtable_file\n"
	"options: -v verbose\n"
	"         -S MySQL Server IP\n"
	"         -p MySQL Port\n"
	"         -s MySQL socket\n"
	"         -D MySQL Database Name\n"
	"         -U MySQL User Name\n"
	"         -P MySQL Password"
	;

static int
get_options(int argc, char **argv, char **mysql_server, char **mysql_socket, char **mysql_port,
		char **mysql_database, char **mysql_user, char **mysql_pass, char ***filename)
{
	int             c;

	*mysql_port = *mysql_server = *mysql_database = *mysql_socket = *mysql_user = *mysql_pass = 0;
	while ((c = getopt(argc, argv, "vS:p:s:D:U:P:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'S':
			*mysql_server = optarg;
			break;
		case 'p':
			*mysql_port = optarg;
			break;
		case 's':
			*mysql_socket = optarg;
			break;
		case 'D':
			*mysql_database = optarg;
			break;
		case 'U':
			*mysql_user = optarg;
			break;
		case 'P':
			*mysql_pass = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		} /*- switch(c) */
	} /*- while ((c = getopt(argc, argv, "vS:p:s:D:U:P:")) != -1) */
	if (optind < argc)
		*filename = argv + optind;
	else {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vtable: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char            inbuf[512];
	char           *mysql_host, *mysql_db, *port, *mysql_sock, *mysql_user, *mysql_pass, *ptr;
	char          **filename, **fptr;
	static stralloc line = {0};
	int             fd, mysql_port, errors, match;
	MYSQL           vmysql;
	struct substdio ssin;

	if (get_options(argc, argv, &mysql_host, &mysql_sock, &port, &mysql_db,
		&mysql_user, &mysql_pass, &filename))
		return (1);
	if (verbose) {
		out("vtable", "connecting to mysql ");
		out("vatable", mysql_host);
		out("vtable", ":");
		out("vtable", port ? port : mysql_sock);
		out("vtable", " db ");
		out("vtable", mysql_db);
		out("vtable", "\n");
		flush("vtable");
	}
	if (!mysql_Init(&vmysql))
		die_nomem();
	if (port)
		scan_uint(port, (unsigned int *) &mysql_port);
	else
		mysql_port = 0;
	if (!(in_mysql_real_connect(&vmysql, mysql_host, mysql_user, mysql_pass,
		mysql_db, mysql_port, mysql_sock, 0))) {
		strerr_warn4("vtable: mysql_real_connect: ", mysql_host, ": ", (char *) in_mysql_error(&vmysql), 0);
		return (1);
	}
	for (errors = 0, fptr = filename; *fptr; fptr++) {
		if (verbose) {
			out("vtable", "processing ");
			out("vtable", *fptr);
			out("vtable", "\n");
			flush("vtable");
		}
		if ((fd = open_read(*fptr)) == -1) {
			strerr_warn3("vtable: open: ", *fptr, ": ", &strerr_sys);
			return (-1);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		mysql_stack("create table IF NOT EXISTS ");
		for(;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("vtable: read: ", *fptr, ": ", &strerr_sys);
				close(fd);
				return (1);
			}
			if (line.len == 0 || !match)
				strerr_warn3("vtable", *fptr, "incomplete line", 0);
			else
			if (match) {
				line.len--;
				line.s[line.len] = 0; /*- remove newline */
			}
			match = str_chr(line.s, '#');
			if (line.s[match])
				line.s[match] = 0;
			for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			mysql_stack(line.s);
			mysql_stack(" ");
		}
		if (!(ptr = mysql_stack(0))) {
			strerr_warn1("vtable: mysql_stack returned NULL", 0);
		} else {
			if (verbose) {
				out("vtable", ptr);
				out("vtable", "\n");
				flush("vtable");
			}
			if (mysql_query(&vmysql, ptr))
				strerr_warn4("vtable: mysql_query: ", ptr, ": ", (char *) in_mysql_error(&vmysql), 0);
		}
		close(fd);
	}
	in_mysql_close(&vmysql);
	return (errors);
}
