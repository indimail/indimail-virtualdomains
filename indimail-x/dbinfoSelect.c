/*
 * $Log: dbinfoSelect.c,v $
 * Revision 1.2  2019-04-20 08:10:35+05:30  Cprogrammer
 * allow negative values for fd
 *
 * Revision 1.1  2019-04-11 07:54:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoSelect.c,v 1.2 2019-04-20 08:10:35+05:30 Cprogrammer Exp mbhangui $";
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
#include <env.h>
#include <str.h>
#include <fmt.h>
#endif
#include "variables.h"
#include "dbload.h"
#include "is_distributed_domain.h"
#include "common.h"

static void
die_nomem()
{
	strerr_warn1("dbinfoSelect: out of memory", 0);
	_exit(111);
}

int
dbinfoSelect(char *filename, char *domain, char *mdahost, int row_format)
{
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	char            strnum[FMT_ULONG];
	int             count, is_dist;
	int             first_flag = 0, i;
	char           *ptr;

	if (filename && *filename && !env_put2("MCDFILE", filename))
		die_nomem();
	if (OpenDatabases())
		return (1);
	for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;(*rhostsptr);mysqlptr++, rhostsptr++, count++) {
		if (mdahost && *mdahost && str_diffn(mdahost, (*rhostsptr)->mdahost, DBINFO_BUFF))
			continue;
		if (domain && *domain && str_diffn(domain, (*rhostsptr)->domain, DBINFO_BUFF))
			continue;
		if (row_format) {
			first_flag++;
			out("dbinfoSelect", (*rhostsptr)->isLocal ? "auto " : "DBINFO ");
			out("dbinfoSelect", " ");
			out("dbinfoSelect", (*rhostsptr)->domain);
			out("dbinfoSelect", " ");
			i = is_distributed_domain((*rhostsptr)->domain);
			if (i == -1)
				out("dbinfoSelect", "-1 ");
			else
			if (!i)
				out("dbinfoSelect", "0 ");
			else
				out("dbinfoSelect", "1 ");
			out("dbinfoSelect", (*rhostsptr)->server);
			out("dbinfoSelect", " ");
			out("dbinfoSelect", (*rhostsptr)->mdahost);
			out("dbinfoSelect", " ");
			if ((*mysqlptr)->unix_socket) {
				out("dbinfoSelect", (*mysqlptr)->unix_socket);
				out("dbinfoSelect", " 0 ");
			} else {
				strnum[fmt_uint(strnum, (*mysqlptr)->port)] = 0;
				out("dbinfoSelect", strnum);
				out("dbinfoSelect", " ");
				strnum[fmt_uint(strnum, (*rhostsptr)->use_ssl)] = 0;
				out("dbinfoSelect", strnum);
				out("dbinfoSelect", " ");
			}
			out("dbinfoSelect", (*rhostsptr)->database);
			out("dbinfoSelect", " ");
			out("dbinfoSelect", (*rhostsptr)->user);
			out("dbinfoSelect", " ");
			if (filename) {
				out("dbinfoSelect", filename);
				out("dbinfoSelect", " ");
			}
			out("dbinfoSelect", (*rhostsptr)->password);
			out("dbinfoSelect", " ");
			out("dbinfoSelect", "\n");
			flush("dbinfoSelect");
			continue;
		}
		if (!first_flag++) {
			out("dbinfoSelect", "MySQL Client Version: ");
			out("dbinfoSelect", (char *) in_mysql_get_client_info());
			out("dbinfoSelect", "\n");
			flush("dbinfoSelect");
		}
		out("dbinfoSelect", "connection to  ");
		out("dbinfoSelect", (char *) in_mysql_get_host_info(*mysqlptr));
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "protocol       ");
		strnum[fmt_uint(strnum, in_mysql_get_proto_info(*mysqlptr))] = 0;
		out("dbinfoSelect", strnum);
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "server version ");
		out("dbinfoSelect",  (char *) in_mysql_get_server_info(*mysqlptr));
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "domain         ");
		out("dbinfoSelect", (*rhostsptr)->domain);
		if ((is_dist = is_distributed_domain((*rhostsptr)->domain)) == -1) {
			out("dbinfoSelect", " - can't figure out dist flag\n");
		} else {
			out("dbinfoSelect", " - ");
			out("dbinfoSelect", is_dist == 1 ? "Distributed\n" : "Non Distributed\n");
		}
		strnum[fmt_uint(strnum, count)] = 0;
		out("dbinfoSelect", "sqlserver[");
		if (count < 10)
			out("dbinfoSelect", "00");
		else
		if (count < 100)
			out("dbinfoSelect", "0");
		out("dbinfoSelect", strnum);
		out("dbinfoSlect", "] ");
		out("dbinfoSelect", (*rhostsptr)->server);
		out("dbinfoSelect", "\n");
		if (*((*rhostsptr)->mdahost)) {
			out("dbinfoSelect", "mda host       ");
			out("dbinfoSelect", (*rhostsptr)->mdahost);
			out("dbinfoSelect", "\n");
		}
		if ((*mysqlptr)->unix_socket) {
			out("dbinfoSelect", "Unix   Socket  ");
			out("dbinfoSelect", (*mysqlptr)->unix_socket);
			out("dbinfoSelect", "\n");
		} else {
			strnum[fmt_uint(strnum, (*rhostsptr)->port)] = 0;
			out("dbinfoSelect", "TCP/IP Port    ");
			out("dbinfoSelect", strnum);
			out("dbinfoSelect", "\n");
			out("dbinfoSelect", "Use SSL        ");
			out("dbinfoSelect", (*rhostsptr)->use_ssl ? "Yes" : "No");
			out("dbinfoSelect", "\n");
			if ((*rhostsptr)->use_ssl) {
   				out("dbinfoSelect", "SSL Cipher     ");
				out("dbinfoSelect", (char *) in_mysql_get_ssl_cipher(*mysqlptr));
				out("dbinfoSelect", "\n");
			}
		}
		out("dbinfoSelect", "database       ");
		out("dbinfoSelect", (*rhostsptr)->database);
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "user           ");
		out("dbinfoSelect", (*rhostsptr)->user);
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "password       ");
		out("dbinfoSelect", (*rhostsptr)->password);
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "fd             ");
		strnum[fmt_int(strnum, (*rhostsptr)->fd)] = 0;
		out("dbinfoSelect", strnum);
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "DBINFO Method  ");
		out("dbinfoSelect", (*rhostsptr)->isLocal ? "Auto\n" : "DBINFO\n");
		if ((*rhostsptr)->fd == -1) {
			out("dbinfoSelect", "MySQL Stat     mysql_real_connect: ");
			out("dbinfoSelect",  (char *) in_mysql_error((*mysqlptr)));
			out("dbinfoSelect", "\n");
		} else
		if ((ptr = (char *) in_mysql_stat((*mysqlptr)))) {
			out("dbinfoSelect", "MySQL Stat     ");
			out("dbinfoSelect", ptr);
		} else {
			out("dbinfoSelect", "MySQL Stat     ");
			out("dbinfoSelect", (char *) in_mysql_error((*mysqlptr)));
		}
		out("dbinfoSelect", "\n");
		out("dbinfoSelect", "--------------------------\n");
		flush("dbinfoSelect");
	}
	close_db();
	return (!first_flag);
}
#endif
