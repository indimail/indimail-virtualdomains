/*
 * $Log: iopen.c,v $
 * Revision 1.5  2019-05-28 23:28:23+05:30  Cprogrammer
 * fixed error message for mysql_real_connect failure
 *
 * Revision 1.4  2019-05-28 17:40:41+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.3  2019-04-22 23:12:41+05:30  Cprogrammer
 * added stdlib.h
 *
 * Revision 1.2  2019-04-17 17:46:50+05:30  Cprogrammer
 * fixed formatting for mysql_real_connect error message
 *
 * Revision 1.1  2019-04-14 18:29:41+05:30  Cprogrammer
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
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#endif
#include "iclose.h"
#include "islocalif.h"
#include "getEnvConfig.h"
#include "variables.h"
#include "set_mysql_options.h"

#ifndef	lint
static char     sccsid[] = "$Id: iopen.c,v 1.5 2019-05-28 23:28:23+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("iopen: out of memory", 0);
	_exit(111);
}

/* 
 * connect to MySQL
 * dbhost = host:user:password:socket/port:ssl
 */
int
iopen(char *dbhost)
{
	static stralloc SqlBuf = {0}, host_path = {0};
	char            inbuf[512], strnum[FMT_ULONG];
	char           *ptr, *mysql_user = 0, *mysql_passwd = 0, *mysql_database = 0,
		           *mysql_socket = 0, *sysconfdir, *controldir, *server;
	int             t, mysqlport = -1, count, protocol, match;
	unsigned int    flags = 0, use_ssl = 0;
	struct substdio ssin;
	int             fd;

	if (is_open == 1)
		return (0);
	/*-
	 * 1. set mysql_host from dbhost if dbhost is not null.
	 * 2. Check Env Variable for MYSQL_HOST
	 * 3. If MYSQL_HOST is not defined check host.mysql in /var/indimail/control
	 * 4. If host.mysql not present then take the value of MYSQL_HOST 
	 *    defined in indimail.h
	 */
	if (dbhost && *dbhost) {
		if (!stralloc_copys(&mysql_host, dbhost) ||
				!stralloc_0(&mysql_host))
			die_nomem();
	} else
	if ((ptr = (char *) env_get("MYSQL_HOST")) != (char *) 0) {
		if (!stralloc_copys(&mysql_host, ptr) ||
				!stralloc_0(&mysql_host))
			die_nomem();
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.mysql", 11) ||
			!stralloc_0(&host_path))
			die_nomem();
	} else {
		if (!stralloc_copys(&host_path, sysconfdir) ||
			!stralloc_append(&host_path, "/") ||
			!stralloc_cats(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.mysql", 11) ||
			!stralloc_0(&host_path))
			die_nomem();
	}
	if (!mysql_host.len && !access(host_path.s, F_OK)) {
		if ((fd = open_read(host_path.s)) == -1)
			strerr_die3sys(111, "iopen: ", host_path.s, ": ");
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &mysql_host, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "iopen: read: ", host_path.s, ": ");
		}
		if (mysql_host.len == 0)
			strerr_warn3("iopen: ", host_path.s, "incomplete line", 0);
		else
		if (match) {
			mysql_host.len--;
			mysql_host.s[mysql_host.len] = 0; /*- remove newline */
		}
		close(fd);
	} else
	if (!mysql_host.len) {
		if (!stralloc_copys(&mysql_host, MYSQL_HOST) ||
				!stralloc_0(&mysql_host))
			die_nomem();
	}
	mysql_Init(&mysql[1]);
	atexit(iclose);
	/*- 
	 * localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock:ssl
	 * localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock:nossl
	 */
	for (count = 0,ptr = mysql_host.s;*ptr;ptr++) {
		if (*ptr == ':') {
			*ptr = 0;
			switch (count++)
			{
			case 0: /*- mysql user */
				if (*(ptr + 1))
					mysql_user = ptr + 1;
				break;
			case 1: /*- mysql passwd */
				if (*(ptr + 1))
					mysql_passwd = ptr + 1;
				break;
			case 2: /*- mysql socket/port */
				if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
					mysql_socket = ptr + 1;
				else
				if (*(ptr + 1))
					indi_port = ptr + 1;
				break;
			case 3: /*- ssl/nossl */
				use_ssl = str_diffn(ptr + 1, "ssl", 4) ? 0 : 1;
				break;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "MYSQL_USER", MYSQL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "MYSQL_PASSWD", MYSQL_PASSWD);
	if (!mysql_socket)
		mysql_socket = (char *) env_get("MYSQL_SOCKET");
	if (!indi_port && !(indi_port = (char *) env_get("MYSQL_VPORT")))
		indi_port = "0";
	getEnvConfigStr(&mysql_database, "MYSQL_DATABASE", MYSQL_DATABASE);
	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	scan_int(indi_port, &mysqlport);
#ifdef CLUSTERED_SITE
	if (!isopen_cntrl || str_diffn(cntrl_host.s, mysql_host.s, mysql_host.len) || str_diffn(cntrl_port, indi_port, FMT_ULONG)) {
#endif
		flags = use_ssl;
		/*- 
		 * mysql_options bug
		 * if MYSQL_READ_DEFAULT_FILE is used
		 * mysql_real_connect fails by connecting with a null unix domain socket
		 */
		if ((count = set_mysql_options(&mysql[1], 
				mysqlport > 0 || mysql_socket ? 0 : "indimail.cnf",
				mysqlport > 0 || mysql_socket ? 0 : "indimail",
				&flags))) {
			strnum[fmt_uint(strnum, count)] = 0;
			strerr_warn4("iopen: mysql_options(", strnum, "): error setting ",
				(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
			return (-1);
		}
		server = (mysql_socket && islocalif(mysql_host.s) ? "localhost" : mysql_host.s);
		/*
		 * MySQL/MariaDB is very stubborn.
		 * It uses Unix domain socket, even if port is set and the socket value is NULL
		 * Force it to use TCP when port is provided and unix_socket is NULL
		 */
		if (mysqlport > 0 && !mysql_socket) {
			protocol = MYSQL_PROTOCOL_TCP;
			if (int_mysql_options(&mysql[1], MYSQL_OPT_PROTOCOL, (char *) &protocol)) {
				strerr_warn2("iopen: mysql_options(MYSQL_OPT_PROTOCOL): ",
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
				return (-1);
			}
		}
		if (!(in_mysql_real_connect(&mysql[1], server, mysql_user, mysql_passwd,
			mysql_database, mysqlport, mysql_socket, flags))) {
			flags = use_ssl;
			if ((count = set_mysql_options(&mysql[1], "indimail.cnf", "indimail", &flags))) {
				strnum[fmt_uint(strnum, count)] = 0;
				strerr_warn4("iopen: mysql_options(", strnum, "): error setting ",
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
				return (-1);
			}
			if ((count = in_mysql_errno(&mysql[1])) != ER_DATABASE_NAME) {
				strerr_warn12("iopen: mysql_real_connect: ", server, "@", mysql_database,
					" user ", mysql_user, " port ", indi_port, " socket ",
					mysql_socket ? mysql_socket : "TCP/IP", ": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			} 
			if (!(in_mysql_real_connect(&mysql[1], server, mysql_user, mysql_passwd, NULL,
				mysqlport, mysql_socket, flags))) {
				strerr_warn10("iopen: mysql_real_connect: ", server, " user ", mysql_user,
					" port ", indi_port, " socket ", mysql_socket ? mysql_socket : "TCP/IP",
					": ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
			if (!stralloc_copyb(&SqlBuf, "CREATE DATABASE ", 16) ||
				!stralloc_cats(&SqlBuf, mysql_database) || !stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("iopen: mysql_query: ", SqlBuf.s, ": ",
					(char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
			if (in_mysql_select_db(&mysql[1], mysql_database)) {
				strerr_warn4("iopen: mysql_select: ", mysql_database, ": ",
					(char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		}
		is_open = 1;
#ifdef CLUSTERED_SITE
	} else {
		mysql[1] = mysql[0];
		mysql[1].affected_rows= ~(my_ulonglong) 0;
		is_open = 2;
	}
#endif
	return (0);
}
