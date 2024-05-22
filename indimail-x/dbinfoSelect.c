/*
 * $Log: dbinfoSelect.c,v $
 * Revision 1.4  2024-05-22 22:36:48+05:30  Cprogrammer
 * fix SIGSEGV when mysql is down
 *
 * Revision 1.3  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
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
static char     sccsid[] = "$Id: dbinfoSelect.c,v 1.4 2024-05-22 22:36:48+05:30 Cprogrammer Exp mbhangui $";
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
#include <subfd.h>
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
			i = is_distributed_domain((*rhostsptr)->domain);
			subprintfe(subfdout, "dbinfoSelect", "%s %s %2d %s %s " ,
					(*rhostsptr)->isLocal ? "auto " : "DBINFO ", (*rhostsptr)->domain, i > 0 ? 1 : i,
					(*rhostsptr)->server, (*rhostsptr)->mdahost);
			if ((*mysqlptr)->unix_socket)
				subprintfe(subfdout, "dbinfoSelect", "%s 0 ", (*mysqlptr)->unix_socket);
			else
				subprintfe(subfdout, "dbinfoSelect", "%u %d ", (*mysqlptr)->port, (*rhostsptr)->use_ssl);
			subprintfe(subfdout, "dbinfoSelect", "%s %s ", (*rhostsptr)->database, (*rhostsptr)->user);
			if (filename)
				subprintfe(subfdout, "dbinfoSelect", "%s ", filename);
			subprintfe(subfdout, "dbinfoSelect", "%s\n", (*rhostsptr)->password);
			flush("dbinfoSelect");
			continue;
		}
		if (!first_flag++)
			subprintfe(subfdout, "dbinfoSelect", "MySQL Client Version: %s\n", (char *) in_mysql_get_client_info());
		subprintfe(subfdout, "dbinfoSelect", "domain         %s", (*rhostsptr)->domain);
		if ((is_dist = is_distributed_domain((*rhostsptr)->domain)) == -1)
			subprintfe(subfdout, "dbinfoSelect", " - can't figure out dist flag\n");
		else
			subprintfe(subfdout, "dbinfoSelect", " - %s\n", is_dist == 1 ? "Distributed" : "Non Distributed");
		subprintfe(subfdout, "dbinfoSelect", "sqlserver[%03d] %s\n", count, (*rhostsptr)->server);
		if ((*mysqlptr)->unix_socket)
			subprintfe(subfdout, "dbinfoSelect", "Unix   Socket  %s\n", (*mysqlptr)->unix_socket);
		else
			subprintfe(subfdout, "dbinfoSelect", "TCP/IP Port    %d\n", (*rhostsptr)->port);
		subprintfe(subfdout, "dbinfoSelect", "database       %s\n", (*rhostsptr)->database);
		subprintfe(subfdout, "dbinfoSelect", "user           %s\n", (*rhostsptr)->user);
		subprintfe(subfdout, "dbinfoSelect", "password       %s\n", (*rhostsptr)->password);
		subprintfe(subfdout, "dbinfoSelect", "fd             %d\n", (*rhostsptr)->fd);
		subprintfe(subfdout, "dbinfoSelect", "DBINFO Method  %s\n", (*rhostsptr)->isLocal ? "Auto" : "DBINFO");
		if (*((*rhostsptr)->mdahost))
			subprintfe(subfdout, "dbinfoSelect", "mda host       %s\n", (*rhostsptr)->mdahost);
		if ((*rhostsptr)->fd == -1) {
			subprintfe(subfdout, "dbinfoSelect", "MySQL Status   mysql_real_connect: %s\n", (char *) in_mysql_error((*mysqlptr)));
			out("dbinfoSelect", "--------------------------\n");
			flush("dbinfoSelect");
			continue;
		}
		if ((ptr = (char *) in_mysql_stat((*mysqlptr))))
			subprintfe(subfdout, "dbinfoSelect", "MySQL Status   %s\n", ptr);
		else
			subprintfe(subfdout, "dbinfoSelect", "MySQL Status   %s\n", (char *) in_mysql_error((*mysqlptr)));
		subprintfe(subfdout, "dbinfoSelect", "connection to  %s\n", (char *) in_mysql_get_host_info(*mysqlptr));
		subprintfe(subfdout, "dbinfoSelect", "protocol       %u\n", in_mysql_get_proto_info(*mysqlptr));
		subprintfe(subfdout, "dbinfoSelect", "server version %s\n", (char *) in_mysql_get_server_info(*mysqlptr));
		subprintfe(subfdout, "dbinfoSelect", "Use SSL        %s\n", (*rhostsptr)->use_ssl ? "Yes" : "No");
		if (!(*mysqlptr)->unix_socket && (*rhostsptr)->use_ssl)
			subprintfe(subfdout, "dbinfoSelect", "SSL Cipher     %s\n", (char *) in_mysql_get_ssl_cipher(*mysqlptr));
		out("dbinfoSelect", "--------------------------\n");
		flush("dbinfoSelect");
	}
	close_db();
	return (!first_flag);
}
#endif
