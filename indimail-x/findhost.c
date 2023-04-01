/*
 * $Log: findhost.c,v $
 * Revision 1.12  2023-04-01 13:29:19+05:30  Cprogrammer
 * display mysql error for mysql_options()
 *
 * Revision 1.11  2023-03-20 09:59:14+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.10  2021-07-27 18:05:27+05:30  Cprogrammer
 * set default domain using vset_default_domain
 *
 * Revision 1.9  2020-04-30 19:26:37+05:30  Cprogrammer
 * changed scope of ssin to local
 *
 * Revision 1.8  2020-04-01 18:54:35+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.7  2019-06-30 10:14:19+05:30  Cprogrammer
 * seperate fields in error string by commas
 *
 * Revision 1.6  2019-06-27 20:00:23+05:30  Cprogrammer
 * provide default cnf file and group to set_mysql_options
 *
 * Revision 1.5  2019-06-27 10:45:31+05:30  Cprogrammer
 * display ssl setting for mysql_real_connect() error
 *
 * Revision 1.4  2019-05-28 17:39:09+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.3  2019-04-22 23:10:42+05:30  Cprogrammer
 * added stdlib.h header
 *
 * Revision 1.2  2019-04-20 08:12:53+05:30  Cprogrammer
 * fixed formatting for error messages
 *
 * Revision 1.1  2019-04-18 15:40:26+05:30  Cprogrammer
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
#ifdef HAVE_QMAIL
#include <strerr.h>
#endif
#include "get_real_domain.h"
#include "create_table.h"
#include "SqlServer.h"
#include "sql_getip.h"
#include "smtp_port.h"
#include "iopen.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: findhost.c,v 1.12 2023-04-01 13:29:19+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("findhost: out of memory", 0);
	_exit(111);
}

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HVE_ERRNO_H
#include <errno.h>
#endif
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <scan.h>
#include <fmt.h>
#include <env.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "set_mysql_options.h"
#include "findhost.h"
#include "vset_default_domain.h"

static char     inbuf[512];
static char     strnum[FMT_ULONG];

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

void
iclose_cntrl()
{
	/*
	 * disconnection from the database
	 */
	if (isopen_cntrl == 1) {
		isopen_cntrl = 0;
		in_mysql_close(&mysql[0]);
	}
}

int
open_central_db(char *dbhost)
{
	struct substdio ssin;
	static stralloc host_path = {0}, SqlBuf = {0};
	static char     atexit_registered = 0;
	char           *ptr, *mysql_user = 0, *mysql_passwd = 0, *mysql_database = 0,
				   *cntrl_socket = 0, *sysconfdir, *controldir;
	int             mysqlport = -1, count, protocol, fd, match;
	unsigned int    flags, use_ssl = 0;

	if (isopen_cntrl == 1)
		return (0);
	/*-
	 * 1. Check Env Variable for CNTRL_HOST
	 * 2. If CNTRL_HOST is not defined check host.cntrl in /var/indimail/control
	 * 3. If host.cntrl is not present check host.mysql in /var/indimail/control
	 * 4. If host.cntrl not present then take the value of CNTRL_HOST
	 *    defined in indimail.h
	 */
	if (dbhost && *dbhost) {
		if (!stralloc_copys(&cntrl_host, dbhost) || !stralloc_0(&cntrl_host))
			die_nomem();
		cntrl_host.len--;
	} else
	if ((ptr = (char *) env_get("CNTRL_HOST")) != (char *) 0) {
		if (!stralloc_copys(&cntrl_host, ptr) || !stralloc_0(&cntrl_host))
			die_nomem();
		cntrl_host.len--;
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.cntrl", 11) ||
				!stralloc_0(&host_path))
			die_nomem();
		if (access(host_path.s, F_OK) && (!stralloc_copys(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.cntrl", 11) ||
				!stralloc_0(&host_path)))
			die_nomem();
	} else {
		if (!stralloc_copys(&host_path, sysconfdir) ||
				!stralloc_append(&host_path, "/") ||
				!stralloc_cats(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.cntrl", 11) ||
				!stralloc_0(&host_path))
			die_nomem();
		if (access(host_path.s, F_OK) && (!stralloc_copys(&host_path, controldir) ||
				!stralloc_catb(&host_path, "/host.cntrl", 11) ||
				!stralloc_0(&host_path)))
			die_nomem();
	}
	if (!cntrl_host.len && !access(host_path.s, F_OK)) {
		if ((fd = open_read(host_path.s)) == -1)
			strerr_die3sys(111, "open_central_db: open: ", host_path.s, ": ");
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &cntrl_host, &match, '\n') == -1)
			strerr_die3sys(111, "open_central_db: read: ", host_path.s, ": ");
		close(fd);
		if (!cntrl_host.len)
			strerr_die3x(100, "open_central_db: ", host_path.s, ": incomplete line");
		if (match) {
			cntrl_host.len--;
			if (!cntrl_host.len)
				strerr_die3x(100, "open_central_db: ", host_path.s, ": incomplete line");
			cntrl_host.s[cntrl_host.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&cntrl_host))
				die_nomem();
			cntrl_host.len--;
		}
	} else
	if (!cntrl_host.len) {
		if (!stralloc_copys(&cntrl_host, CNTRL_HOST) || !stralloc_0(&cntrl_host))
			die_nomem();
	}
	mysql_Init(&mysql[0]);
	if (!atexit_registered++)
		atexit(iclose_cntrl);
#ifdef HAVE_LOCAL_INFILE
	if (in_mysql_options(&mysql[0], MYSQL_OPT_LOCAL_INFILE, 0))
		strerr_warn1("open_central_db: mysql_options: MYSQL_OPT_LOCAL_INFILE: unknown option", 0);
#endif
	for (count = 0, ptr = cntrl_host.s;*ptr;ptr++) {
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
					cntrl_socket = ptr + 1;
				else
				if (*(ptr + 1))
					cntrl_port = ptr + 1;
				break;
			case 3: /*- ssl/nossl */
				use_ssl = (str_diffn(ptr + 1, "ssl", 4) ? 0 : 1);
				break;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "CNTRL_USER", CNTRL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "CNTRL_PASSWD", CNTRL_PASSWD);
	if (!cntrl_socket)
		cntrl_socket = (char *) env_get("CNTRL_SOCKET");
	if (!cntrl_port && !(cntrl_port = (char *) env_get("CNTRL_VPORT")))
		cntrl_port = "0";
	getEnvConfigStr(&mysql_database, "CNTRL_DATABASE", CNTRL_DATABASE);
	getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
	scan_int(cntrl_port, &mysqlport);
	/*
	 * is_open == 0 &&  cntrl_host == mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 0 &&  cntrl_host != mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 0 &&  cntrl_host == mysql_host && cntrl_port != indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 1 &&  cntrl_host == mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> use mysql_host
	 * is_open == 1 &&  cntrl_host != mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 1 &&  cntrl_host == mysql_host && cntrl_port != indi_port -> connect to cntrl_host -> connect cntrl_host
	 */
	if (!is_open || str_diffn(cntrl_host.s, mysql_host.s, mysql_host.len) || str_diffn(cntrl_port, indi_port, FMT_ULONG)) {
		flags = use_ssl;
		/*-
		 * mysql_options bug
		 * if MYSQL_READ_DEFAULT_FILE is used
		 * mysql_real_connect fails by connecting with a null unix domain socket
		 */
		if ((count = set_mysql_options(&mysql[0], "indimail.cnf", "indimail", &flags))) {
			strnum[fmt_uint(strnum, count)] = 0;
			strerr_warn6("open_central_db: mysql_options(", strnum, "): ",
				(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", ": ", in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
		/*
		 * MySQL/MariaDB is very stubborn.
		 * It uses Unix domain socket, even if port is set and the socket value is NULL
		 * Force it to use TCP when port is provided and unix_socket is NULL
		 */
		if (mysqlport > 0 && !cntrl_socket) {
			protocol = MYSQL_PROTOCOL_TCP;
			if (int_mysql_options(&mysql[1], MYSQL_OPT_PROTOCOL, (char *) &protocol)) {
				strerr_warn2("open_central_db: mysql_options(MYSQL_OPT_PROTOCOL): ",
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", 0);
				return (-1);
			}
		}
		if (!(in_mysql_real_connect(&mysql[0], cntrl_host.s, mysql_user, mysql_passwd,
					mysql_database, mysqlport, cntrl_socket, flags))) {
			flags = use_ssl;
			if ((count = set_mysql_options(&mysql[0], "indimail.cnf", "indimail", &flags))) {
				strnum[fmt_uint(strnum, count)] = 0;
				strerr_warn5("open_central_db: mysql_options(", strnum,
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error", ": ", in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
			if (!(in_mysql_real_connect(&mysql[0], cntrl_host.s, mysql_user, mysql_passwd,
					NULL, mysqlport, cntrl_socket, flags))) {
				strerr_warn10("open_central_db: mysql_real_connect: ", cntrl_host.s, " user ", mysql_user,
					", port ", cntrl_port, ", socket ", cntrl_socket ? cntrl_socket : "TCP/IP",
					!cntrl_socket && use_ssl ? ": use_ssl=1: " : ": use_ssl=0: ",
					(char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
			if (!stralloc_copys(&SqlBuf, "create database ") ||
				!stralloc_cats(&SqlBuf, mysql_database) || !stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("open_central_db: mysql_query [", SqlBuf.s, "]: ",
					(char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
			if (in_mysql_select_db(&mysql[0], mysql_database)) {
				strerr_warn4("open_central_db: mysql_select_db: ", mysql_database, ": ",
					(char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
		}
		isopen_cntrl = 1;
	} else {
		mysql[0] = mysql[1];
		mysql[0].affected_rows= ~(my_ulonglong) 0;
		isopen_cntrl = 2; /*- same connection as from host.mysql */
	}
	return (0);
}

/*
 * connect_primarydb
 * 0 - do not connect, and look for '*' if email does not exist
 * 1 - connect, and look for '*' if email does not exist
 * 2 - do not connect and do not look for '*'
 * 3 - connect and do not look for '*'
 */
char           *
findhost(char *email, int connect_primarydb)
{
	static stralloc mailhost = {0}, prevEmail = {0},
					user = {0}, domain = {0}, SqlBuf = {0}, hostid = {0};
	char           *ptr, *real_domain, *ip_addr;
	int             len, port, err, attempt = 0;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!email || !*email) {
		userNotFound = 1;
		return ((char *) 0);
	}
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE") && mailhost.len && !str_diffn(email, prevEmail.s, prevEmail.len + 1))
		return (mailhost.s);
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	mailhost.len = user.len = domain.len = 0;
	real_domain = (char *) 0;
	userNotFound = 0;
	if (open_central_db(0)) /*- open connection to mysql (cntrl_host or assign opened connection to primary db */
		return ((char *) 0);
	if (!stralloc_copys(&user, email) || !stralloc_0(&user))
		die_nomem();
	len = str_chr(user.s, '@');
	if (user.s[len]) {
		if (user.s[len + 1]) {
			if (!stralloc_copys(&domain, user.s + len + 1) || !stralloc_0(&domain))
				die_nomem();
			domain.len--;
			if (!(real_domain = get_real_domain(domain.s)))
				return ((char *) 0);
		} else {
			ptr = vset_default_domain();
			if (!stralloc_copys(&domain, ptr) || !stralloc_0(&domain))
				die_nomem();
			real_domain = domain.s;
		}
		user.s[len] = 0;
	} else {
		ptr = vset_default_domain();
		if (!stralloc_copys(&domain, ptr) || !stralloc_0(&domain))
			die_nomem();
		domain.len--;
		real_domain = domain.s;
	}
	if (!*real_domain)
		return ((char *) 0);
	/* get data from hostcntrl */
	if (!stralloc_copyb(&SqlBuf, "select high_priority host from ", 31) ||
		!stralloc_cats(&SqlBuf, cntrl_table) ||
		!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
		!stralloc_catb(&SqlBuf, user.s, len) ||
		!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
		!stralloc_cats(&SqlBuf, real_domain) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
again:
	if (attempt != -1)
		attempt++;
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		err = in_mysql_errno(&mysql[0]);
		strerr_warn4("findhost: mysql_query [", SqlBuf.s, "]: ",
			(char *) in_mysql_error(&mysql[0]), 0);
		if (err == ER_NO_SUCH_TABLE && create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT)) {
			strerr_warn6("findhost: create_table: ", cntrl_table, ": ", SqlBuf.s, ": ",
				(char *) in_mysql_error(&mysql[0]), 0);
		}
		if (err == ER_NO_SUCH_TABLE || err == ER_SYNTAX_ERROR)
			userNotFound = 1;
		/*- reconnect to MySQL if server gone away */
		if (in_mysql_ping(&mysql[0])) {
			if (attempt == 1) {
				iclose_cntrl();
				goto again;
			}
		}
		return ((char *) 0);
	} else {
		if (attempt != -1)
			attempt = 1;
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("findhost: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (in_mysql_num_rows(res) == 0) {
		if (connect_primarydb > 1) {
			userNotFound = 1;
			in_mysql_free_result(res);
			return ((char *) 0);
		}
		/*- connect_primarydb == 2 or connect_primarydb == 3 */
		if (attempt == 1) {
			in_mysql_free_result(res);
			/*- look for default entry (pw_name = '*' */
			if (!stralloc_copyb(&SqlBuf, "select high_priority host from ", 31) ||
				!stralloc_cats(&SqlBuf, cntrl_table) ||
				!stralloc_catb(&SqlBuf, " where pw_name=\"*\" and pw_domain=\"", 34) ||
				!stralloc_cats(&SqlBuf, real_domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
				die_nomem();
			attempt = -1;
			goto again;
		}
		userNotFound = 1;
		in_mysql_free_result(res);
		return ((char *) 0);
	}
	row = in_mysql_fetch_row(res);
	if (!(ip_addr = sql_getip(row[0]))) {
		strerr_warn2("findhost: sql_getip: ", (char *) in_mysql_error(&mysql[0]), 0);
		in_mysql_free_result(res);
		return ((char *) 0);
	}
	if (!stralloc_copys(&hostid, row[0]) || !stralloc_0(&hostid))
		die_nomem();
	in_mysql_free_result(res);
	if (connect_primarydb == 1 || connect_primarydb == 3) {
		if (!(ptr = SqlServer(ip_addr, real_domain))) {
			strerr_warn4("findhost: SqlServer: Unable to find SqlServer IP for mailstore ",
				ip_addr, ", ", real_domain, 0);
			return ((char *) 0);
		}
	 	/* host:user:password:socket/port:ssl */
		if (iopen(ptr)) {/*- connect to primary db */
			strerr_warn2("findhost: unable to connect to ", ptr, 0);
			return ((char *) 0);
		}
	}
	if ((port = smtp_port(0, real_domain, hostid.s)) == -1) {
		strerr_warn4("findhost: failed to get smtp port for ", real_domain, " ", hostid.s, 0);
		return ((char *) 0);
	} else
	if (!port)
		port = PORT_QMTP;
	if (!stralloc_copys(&prevEmail, email) || !stralloc_0(&prevEmail))
		die_nomem();
	prevEmail.len--;
	if (!stralloc_copy(&mailhost, &domain) ||
		!stralloc_append(&mailhost, ":") ||
		!stralloc_cats(&mailhost, ip_addr) ||
		!stralloc_append(&mailhost, ":") ||
		!stralloc_catb(&mailhost, strnum, fmt_uint(strnum, port)) ||
		!stralloc_0(&mailhost))
		die_nomem();
	mailhost.len--;
	return (mailhost.s);
}

#ifdef QUERY_CACHE
void
findhost_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#else
char           *
findhost(char *email, int connect_primarydb)
{
	char           *domain, ;
	static stralloc tmpbuf = {0}, mailstore = {0};

	if (!email || !*email)
		return ((char *) 0);
	*tmpbuf = *domain = 0;
	if (!stralloc_copys(&tmpbuf, email) || !stralloc_0(&tmpbuf))
		die_nomem();
	tmpbuf.len--;
	i = str_chr(tmpbuf.s, '@');
	if (!tmpbuf.s[i])
		return ((char *) 0);
	domain = tmpbuf.s + i + 1;
	if (!*domain)
		return ((char *) 0);
	if (!stralloc_copyb(&mailstore, domain, tmpbuf.len - (i + 1))) /*- copy without null terminator */
		die_nomem();
	if (!stralloc_catb(&mailstore, ":localhost:25", 13) || !stralloc_0(&mailstore))
		die_nomem();
	if (connect_primarydb)
		iopen(0);
	return (mailstore.s);
}
#endif
