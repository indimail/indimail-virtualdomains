/*
 * $Log: RemoteBulkMail.c,v $
 * Revision 1.5  2020-04-01 18:57:45+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.4  2019-06-30 10:14:30+05:30  Cprogrammer
 * seperate fields in error string by commas
 *
 * Revision 1.3  2019-06-27 20:00:34+05:30  Cprogrammer
 * provide default cnf file and group to set_mysql_options
 *
 * Revision 1.2  2019-06-27 10:45:55+05:30  Cprogrammer
 * display ssl setting for mysql_real_connect() error
 *
 * Revision 1.1  2019-04-15 11:46:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <strerr.h>
#include <scan.h>
#include <str.h>
#include <fmt.h>
#include <getEnvConfig.h>
#endif
#include "iopen.h"
#include "set_mysql_options.h"
#include "create_table.h"
#include "CopyEmailFile.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: RemoteBulkMail.c,v 1.5 2020-04-01 18:57:45+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("RemoteBulkMail: out of memory", 0);
	_exit(111);
}

static MYSQL   *
bulk_host_connect()
{
	char           *bulk_host, *bulk_user = 0, *bulk_passwd = 0, *bulk_database,
				   *bulk_socket = 0, *port = 0, *ptr;
	int             bulk_port, count, protocol;
	char            strnum[FMT_ULONG];
	unsigned int    flags, use_ssl = 0;
	static MYSQL    bulkMySql;

	if ((bulk_host = (char *) env_get("BULK_HOST")) == (char *) 0)
		return (&mysql[1]);
	else {
		for (count = 0,ptr = bulk_host;*ptr;ptr++) {
			if (*ptr == ':') {
				*ptr = 0;
				switch (count++)
				{
				case 0: /*- mysql user */
					if (*(ptr + 1))
						bulk_user = ptr + 1;
					break;
				case 1: /*- mysql passwd */
					if (*(ptr + 1))
						bulk_passwd = ptr + 1;
					break;
				case 2: /*- mysql socket/port */
					if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
						bulk_socket = ptr + 1;
					else
					if (*(ptr + 1))
						port = ptr + 1;
					break;
				case 3: /*- ssl/nossl */
					use_ssl = (str_diffn(ptr + 1, "ssl", 4) ? 0 : 1);
					break;
				}
			}
		}
		if (!bulk_user)
			getEnvConfigStr(&bulk_user, "BULK_USER", MYSQL_USER);
		if (!bulk_passwd)
			getEnvConfigStr(&bulk_passwd, "BULK_PASSWD", MYSQL_PASSWD);
		getEnvConfigStr(&bulk_database, "BULK_DATABASE", MYSQL_DATABASE);
		if (!bulk_socket)
			bulk_socket = (char *) env_get("BULK_SOCKET");
		if (!port && !(port = (char *) env_get("BULK_VPORT")))
			port = "0";
		mysql_Init(&bulkMySql);
		flags = use_ssl;
		/*- 
		 * mysql_options bug
		 * if MYSQL_READ_DEFAULT_FILE is used
		 * mysql_real_connect fails by connecting with a null unix domain socket
		 */
		scan_uint(port, (unsigned int *) &bulk_port);
		if ((count = set_mysql_options(&mysql[1], "indimail.cnf", "indimail", &flags))) {
			strnum[fmt_uint(strnum, count)] = 0;
			strerr_warn4("mysql_options(", strnum, "): ",
				(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
			return ((MYSQL *) 0);
		}
		/*-
		 * MySQL/MariaDB is very stubborn.
		 * It uses Unix domain socket, even if port is set and the socket value is NULL
		 * Force it to use TCP when port is provided and unix_socket is NULL
		 */
		if (bulk_port > 0 && !bulk_socket) {
			protocol = MYSQL_PROTOCOL_TCP;
			if (int_mysql_options(&mysql[1], MYSQL_OPT_PROTOCOL, (char *) &protocol)) {
				strerr_warn2("mysql_options(MYSQL_OPT_PROTOCOL): ",
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
				return ((MYSQL *) 0);
			}
		}
		if ((in_mysql_real_connect(&bulkMySql, bulk_host, bulk_user, bulk_passwd,
				bulk_database, bulk_port, bulk_socket, flags)))
			return (&bulkMySql);
		else
			strerr_warn12("bulk_host_connect: mysql_real_connect: ", bulk_database, "@", bulk_host,
				", user ", bulk_user, ", port ", port, ", socket ",
				bulk_socket ? bulk_socket : "TCP/IP",
				!bulk_socket && use_ssl ? ": use_ssl=1: " : ": use_ssl=0: ",
				(char *) in_mysql_error(&bulkMySql), 0);
			return ((MYSQL *) 0);
	}
}

int
RemoteBulkMail(const char *email, const char *domain, const char *homedir)
{
	static stralloc SqlBuf = {0}, bulkdir = {0}, TmpBuf = {0};
	struct stat     statbuf;
	MYSQL_ROW       row;
	MYSQL_RES      *res;
	MYSQL          *mysqlptr;
	char           *ptr;
	int             status, len;

	if (iopen((char *) 0))
		return (1);
	if (!(mysqlptr = bulk_host_connect()))
		return (1);
	if (!stralloc_copyb(&SqlBuf, "select high_priority filename from bulkmail where emailid=\"", 59) ||
			!stralloc_cats(&SqlBuf, (char *) email) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(mysqlptr, SqlBuf.s)) {
		if (in_mysql_errno(mysqlptr) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "bulkmail", BULKMAIL_TABLE_LAYOUT);
			return (0);
		}
		strerr_warn4("RemoteBulkMail: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(mysqlptr), 0);
		return (1);
	}
	if (!(res = in_mysql_store_result(mysqlptr))) {
		strerr_warn2("RemoteBulkMail: in_mysql_store_result: ", (char *) in_mysql_error(mysqlptr), 0);
		return (1);
	}
	if (!in_mysql_num_rows(res)) {
		in_mysql_free_result(res);
		return (0);
	}
	if (!stralloc_copys(&bulkdir, CONTROLDIR) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (char *) domain) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (ptr = env_get("BULK_MAILDIR")) ? ptr : BULK_MAILDIR) ||
			!stralloc_0(&bulkdir))
		die_nomem();
	bulkdir.len--;
	if (!stralloc_copy(&TmpBuf, &bulkdir) ||
			!stralloc_append(&TmpBuf, "/"))
		die_nomem();
	len = TmpBuf.len;
	for (status = 0; (row = in_mysql_fetch_row(res));) {
		if (!stralloc_cats(&TmpBuf, row[0]) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		TmpBuf.len = len;
		if (stat(TmpBuf.s, &statbuf))
			continue;
		if (CopyEmailFile(homedir, TmpBuf.s, email, 0, 0, 0, 0, 1, statbuf.st_size))
			status = 1;
	}
	in_mysql_free_result(res);
	if (!stralloc_copyb(&SqlBuf, "delete low_priority from bulkmail where emailid=\"", 49) ||
			!stralloc_cats(&SqlBuf, (char *) email) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(mysqlptr, SqlBuf.s)) {
		strerr_warn4("RemoteBulkMail: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(mysqlptr), 0);
		return (1);
	}
	return (status);
}
