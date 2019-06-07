/*
 * $Log: dbload.c,v $
 * Revision 1.4  2019-06-07 16:10:55+05:30  mbhangui
 * fix for missing mysql_get_option() in new versions of libmariadb
 *
 * Revision 1.3  2019-05-28 17:37:26+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.2  2019-04-20 08:11:34+05:30  Cprogrammer
 * BUG: Fixed sigsegv
 *
 * Revision 1.1  2019-04-14 18:02:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <alloc.h>
#include <stralloc.h>
#include <fmt.h>
#include <str.h>
#include <strerr.h>
#include "LoadDbInfo.h"
#include "getEnvConfig.h"
#include "set_mysql_options.h"
#include "islocalif.h"
#include "variables.h"
#include "common.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: dbload.c,v 1.4 2019-06-07 16:10:55+05:30 mbhangui Exp mbhangui $";
#endif

static MYSQL   *is_duplicate_conn(MYSQL **, DBINFO **);
int             int_mysql_options(MYSQL *, enum mysql_option, const void *);

static void
die_nomem()
{
	strerr_warn1("dbload: out of memory", 0);
	_exit(111);
}

static stralloc mcdFile = {0};
int
connect_db(DBINFO **ptr, MYSQL **mysqlptr)
{
	char           *sysconfdir, *controldir, *mcdfile, *server, *str;
	int             maxattempts, retry_interval, count, protocol;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], strnum3[FMT_ULONG];
#ifdef HAVE_MYSQL_OPT_SSL_ENFORCE
	int             use_ssl = 0;
#endif
	unsigned int    flags;

	if ((*ptr)->failed_attempts) {
		getEnvConfigInt(&maxattempts, "MAX_FAIL_ATTEMPTS", MAX_FAIL_ATTEMPTS);
		if ((*ptr)->failed_attempts > maxattempts) {
			getEnvConfigInt(&retry_interval, "MYSQL_RETRY_INTERVAL", MYSQL_RETRY_INTERVAL);
			/*- Do not attempt reconnection to MySQL till this period is over */
			if ((time(0) - (*ptr)->last_attempted) < retry_interval) {
				strnum1[fmt_uint(strnum1, (*ptr)->port)] = 0;
				strnum2[fmt_uint(strnum2, (*ptr)->failed_attempts)] = 0;
				strnum3[fmt_ulong(strnum3, retry_interval - time(0) + (*ptr)->last_attempted)] = 0;
				strerr_warn15("dbload: connect_db: ", (*ptr)->database,
						"@", (*ptr)->server, ": domain ", (*ptr)->domain, " port ", strnum1,
						": got failed attempts [", strnum2, "], retry connect after ",
						strnum3, " secs [", (*ptr)->last_error ? (*ptr)->last_error : "?", "]", 0);
				return 1;
			}
		}
	}
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.') {
		if (!stralloc_copys(&mcdFile, mcdfile))
			die_nomem();
		else
		if (!stralloc_0(&mcdFile))
			die_nomem();
	} else {
		if (*controldir == '/') {
			if (!stralloc_copys(&mcdFile, controldir))
				die_nomem();
			else
			if (!stralloc_append(&mcdFile, "/"))
				die_nomem();
			else
			if (!stralloc_cats(&mcdFile, mcdfile))
				die_nomem();
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (!stralloc_copys(&mcdFile, sysconfdir))
				die_nomem();
			else
			if (!stralloc_append(&mcdFile, "/"))
				die_nomem();
			else
			if (!stralloc_cats(&mcdFile, controldir))
				die_nomem();
			else
			if (!stralloc_append(&mcdFile, "/"))
				die_nomem();
			else
			if (!stralloc_cats(&mcdFile, mcdfile))
				die_nomem();
		}
	}
	if (!mysql_Init(*mysqlptr))
		strerr_die4x(111, "MySQL Init Error: ", (*ptr)->database, "@", (*ptr)->server);
	flags = (*ptr)->use_ssl;
	/*- 
	 * mysql_options bug
	 * if MYSQL_READ_DEFAULT_FILE is used
	 * mysql_real_connect fails by connecting with a null unix domain socket
	 */
	if ((count = set_mysql_options(*mysqlptr, 
		(*ptr)->port > 0 || (*ptr)->socket ? 0 : "indimail.cnf",
		(*ptr)->port > 0 || (*ptr)->socket ? 0 : "indimail",
		&flags)))
	{
		strnum1[fmt_uint(strnum1, count)] = 0;
		strerr_die4x(111, "mysql_options(", strnum1, "): ", (str = error_mysql_options_str(count)) ? str : "unknown error");
	}
	server = ((*ptr)->socket && islocalif((*ptr)->server) ? "localhost" : (*ptr)->server);
	/*
	 * MySQL/MariaDB is very stubborn.
	 * It uses Unix domain socket, even if port is set and the socket value is NULL
	 * Force it to use TCP when port is provided and unix_socket is NULL
	 */
	if ((*ptr)->port > 0 && !(*ptr)->socket) {
		protocol = MYSQL_PROTOCOL_TCP;
		if (int_mysql_options(*mysqlptr, MYSQL_OPT_PROTOCOL, (char *) &protocol))
			strerr_die2x(111, "mysql_options(MYSQL_OPT_PROTOCOL): ",
				(str = error_mysql_options_str(count)) ? str : "unknown error");
	}
	(*ptr)->last_attempted = time(0);
	if (!in_mysql_real_connect(*mysqlptr, server, (*ptr)->user,
			(*ptr)->password, (*ptr)->database, (*ptr)->port, (*ptr)->socket, flags))
	{
		char           *my_error;
		int             my_error_len;

		if ((my_error = (char *) in_mysql_error(*mysqlptr))) {
			my_error_len = str_len(my_error) + 1;
			if (!alloc_re((char *) &((*ptr)->last_error), (*ptr)->last_error ? (str_len((*ptr)->last_error) + 1) : 0,
						my_error_len * sizeof(char))) {
				strnum1[fmt_uint(strnum1, my_error_len)] = 0;
				strerr_die3sys(111, "connect_db: alloc_re", strnum1, " Bytes: ");
			}
			str_copyb((*ptr)->last_error, my_error, my_error_len);
		}
		if (verbose) {
			strnum1[fmt_uint(strnum1, (*ptr)->port)] = 0;
			strerr_warn14("dbload: MySQLConnect: ", (*ptr)->database, "@", server,
					": domain ", (*ptr)->domain, " user ", (*ptr)->user, " port ", strnum1,
					" socket ", (*ptr)->socket ? (*ptr)->socket : "TCP/IP", ": ",
					(char *) in_mysql_error(*mysqlptr), 0);
		}
		(*ptr)->failed_attempts++;
		return (1);
	}
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION) && defined(HAVE_MYSQL_OPT_SSL_ENFORCE)
	if (in_mysql_get_option(*mysqlptr, MYSQL_OPT_SSL_ENFORCE, &use_ssl)) {
		strerr_warn2("dbload: mysql_get_option: MYSQL_OPT_SSL_ENFORCE: ", (char *) in_mysql_error(*mysqlptr), 0);
		return (1);
	}
	(*ptr)->use_ssl = use_ssl;
#endif
	(*ptr)->failed_attempts = 0;
	if ((*ptr)->last_error) {
		alloc_free((*ptr)->last_error);
		(*ptr)->last_error = 0;
	}
	return (0);
}

void
close_db()
{
	MYSQL         **mysqlptr, **mptr;
	MYSQL          *tptr; /*- for temp storage of address */
	DBINFO        **rhostsptr, **rptr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if (!RelayHosts || !MdaMysql)
		return;
	/*
	 * Find out entries for mysqlptr which are duplicate -
	 * assigned by is_duplicate_conn()
	 * to ensure free() and mysql_close() is done only once
	 */
	for (mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++) {
		if ((tptr = (*mysqlptr))) {
			if (verbose) {
				strnum1[fmt_uint(strnum1, (*rhostsptr)->fd)] = 0;
				strnum2[fmt_uint(strnum2, (*rhostsptr)->port)] = 0;
				strerr_warn10("dbload: closing connection fd", strnum1, " database ",
						(*rhostsptr)->database, ", server ", (*rhostsptr)->server,
						", user ", (*rhostsptr)->user, ", port ", strnum2, 0);
			}
			in_mysql_close(*mysqlptr);
			alloc_free((char *) *mysqlptr);
			for (rptr = RelayHosts, mptr = MdaMysql;*rptr;rptr++, mptr++) {
				if ((*mptr) && (*mptr) == tptr)
					(*mptr) = (MYSQL *) 0;
			}
		}
		alloc_free((char *) *rhostsptr);
	}
	alloc_free((char *) MdaMysql);
	alloc_free((char *) RelayHosts);
	RelayHosts = (DBINFO **) 0;
	MdaMysql = (MYSQL **) 0;
	return;
}

int
OpenDatabases()
{
	static MYSQL    dummy; /*- some bug in mysql. Do mysql_init() to prevent it */
	MYSQL         **mysqlptr;
	DBINFO        **ptr;
	static int      total;
	int             count, idx;
	int             fd[3];
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], strnum3[FMT_ULONG];
	extern int      loadDbinfoTotal();

	if (RelayHosts)
		total = loadDbinfoTotal();
	else
	if (!(RelayHosts = LoadDbInfo_TXT(&total))) {
		strerr_warn1("dbload: LoadDbInfo_TXT: ", &strerr_sys);
		return (-1);
	}
	if (!MdaMysql) {
		if (!mysql_Init(&dummy)) {
			strerr_warn1("MYSQL Init Error: ", &strerr_sys);
			return (1);
		}
		if (!total && !(total = loadDbinfoTotal())) {
			strerr_warn1("dbload: loadDbinfoTotal: unable to figure out totals", 0);
			return (1);
		}
		if (!(MdaMysql = (MYSQL **) alloc(sizeof(MYSQL *) * (total)))) {
			strnum1[fmt_uint(strnum1, total * (int) sizeof(MYSQL *))] = 0;
			strerr_warn3("dbload: alloc: ", strnum1, " Bytes: ", &strerr_sys);
			return (-1);
		}
		fd[0] = fd[1] = fd[2] = -1;
		for (count = idx = 0; idx < 3 && count < 2; idx++) {
			if ((count = fd[idx] = open("/dev/null", O_RDONLY)) > 2) {
				close(count);
				fd[idx] = -1;
				break;
			}
		}
		for (count = 1, mysqlptr = MdaMysql, ptr = RelayHosts; (*ptr); ptr++, mysqlptr++, count++) {
			if (!((*mysqlptr) = is_duplicate_conn(mysqlptr, ptr))) {
				if (!((*mysqlptr) = (MYSQL *) alloc(sizeof(MYSQL)))) {
					strnum1[fmt_uint(strnum1, (int) sizeof(MYSQL *))] = 0;
					strerr_warn3("dbload: alloc: ", strnum1, " Bytes: ", &strerr_sys);
					(*ptr)->fd = -1;
					return (-1);
				}
				(*ptr)->failed_attempts = 0;
				(*ptr)->last_error = 0;
				if (connect_db(ptr, mysqlptr)) {
					strnum1[fmt_uint(strnum1, count)] = 0;
					strnum2[fmt_uint(strnum2, (*ptr)->port)] = 0;
					strerr_warn12("dbload: ", strnum1, ": ", (*ptr)->domain,
							" failed db ", (*ptr)->database, "@", (*ptr)->server,
							" for user", (*ptr)->user, " port ", strnum2, 0);
					(*ptr)->fd = -1;
					continue;
				} else
					(*ptr)->fd = (*mysqlptr)->net.fd;
				if (verbose) {
					strnum1[fmt_uint(strnum1, count)] = 0;
					strnum2[fmt_uint(strnum2, (*ptr)->fd)] = 0;
					strnum3[fmt_uint(strnum3, (*ptr)->port)] = 0;
					out("dbload", "connection ");
					out("dbload", strnum1);
					out("dbload", " fd ");
					out("dbload", strnum2);
					out("dbload", ": ");
					out("dbload", (*ptr)->domain);
					out("dbload", " database ");
					out("dbload", (*ptr)->database);
					out("dbload", " server ");
					out("dbload", (*ptr)->server);
					out("dbload", " user ");
					out("dbload", (*ptr)->user);
					out("dbload", " ");
					out("dbload", strnum3);
					out("dbload", "\n");
					flush("dbload");
				}
			} else {
				(*ptr)->fd = (*mysqlptr)->net.fd;
				if (verbose) {
					strnum1[fmt_uint(strnum1, count)] = 0;
					strnum2[fmt_uint(strnum2, (*ptr)->fd)] = 0;
					strnum3[fmt_uint(strnum3, (*ptr)->port)] = 0;
					out("dbload", "connection ");
					out("dbload", strnum1);
					out("dbload", " fd ");
					out("dbload", strnum2);
					out("dbload", ": ");
					out("dbload", (*ptr)->domain);
					out("dbload", " database ");
					out("dbload", (*ptr)->database);
					out("dbload", " server ");
					out("dbload", (*ptr)->server);
					out("dbload", " user ");
					out("dbload", (*ptr)->user);
					out("dbload", " ");
					out("dbload", strnum3);
					out("dbload", "\n");
					flush("dbload");
				}
			}
		}
	} 
	return (0);
}

static MYSQL   *
is_duplicate_conn(MYSQL **mysqlptr, DBINFO **rhostsptr)
{
	DBINFO        **ptr;
	MYSQL         **mptr;

	if (rhostsptr == RelayHosts) /*- How can the first entry be duplicate */
		return ((MYSQL *) 0);
	for (ptr = RelayHosts, mptr = MdaMysql;ptr != rhostsptr;ptr++, mptr++) {
		if (str_diffn((*ptr)->server, (*rhostsptr)->server, DBINFO_BUFF)) {
			/*
			 * if servers do not match but if both
			 * servers are local, then use
			 * first opened mysql connection
			 * NOTE: might be a problem if two
			 * different mysql server run on two different
			 * local interfaces.
			 */
			if (!islocalif((*ptr)->server) || !islocalif((*rhostsptr)->server))
				continue;
		}
		if (str_diffn((*ptr)->database, (*rhostsptr)->database, DBINFO_BUFF))
			continue;
		else
		if (str_diffn((*ptr)->user, (*rhostsptr)->user, DBINFO_BUFF))
			continue;
		else
		if (str_diffn((*ptr)->password, (*rhostsptr)->password, DBINFO_BUFF))
			continue;
		else
		if ((*ptr)->port != (*rhostsptr)->port)
			continue;
		return ((*mptr));
	}
	return ((MYSQL *) 0);
}
