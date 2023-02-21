/*
 * $Log: ProcessInFifo.c,v $
 * Revision 1.14  2022-08-04 14:41:01+05:30  Cprogrammer
 * fetch scram password
 *
 * Revision 1.13  2022-07-04 22:23:56+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 1.12  2021-07-28 12:19:56+05:30  Cprogrammer
 * shortened display in logs on signal
 *
 * Revision 1.11  2021-07-27 18:10:43+05:30  Cprogrammer
 * use getEnvConfigStr to set default domain
 *
 * Revision 1.10  2021-06-09 17:04:06+05:30  Cprogrammer
 * BUG: Fixed read failing on fifo because of O_NDELAY flag
 *
 * Revision 1.9  2021-02-07 19:54:52+05:30  Cprogrammer
 * respond to TCP/IP request when run under tcpserver
 *
 * Revision 1.8  2020-10-11 23:13:41+05:30  Cprogrammer
 * replace deprecated sys_siglist with strsignal
 *
 * Revision 1.7  2020-10-01 18:27:59+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.6  2020-09-17 14:48:15+05:30  Cprogrammer
 * FreeBSD fix for missing tdestroy
 *
 * Revision 1.5  2020-04-01 18:57:29+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.4  2019-05-28 17:41:50+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.3  2019-05-02 14:37:54+05:30  Cprogrammer
 * added argument to specify a 'where clause'
 *
 * Revision 1.2  2019-04-22 23:18:33+05:30  Cprogrammer
 * replaced exit with _exit
 *
 * Revision 1.1  2019-04-20 08:27:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <mysql.h>
#include "load_mysql.h"
#include <mysqld_error.h>
#if defined( HAVE_STRSIGNAL) && defined(HAVE_STRING_H)
#include <string.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <getln.h>
#include <strerr.h>
#include <sig.h>
#include <open.h>
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <error.h>
#include <alloc.h>
#include <str.h>
#include <scan.h>
#include <env.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <getEnvConfig.h>
#endif
#include "in_bsearch.h"
#include "common.h"
#include "iopen.h"
#include "iclose.h"
#include "create_table.h"
#include "variables.h"
#include "get_indimailuidgid.h"
#include "r_mkdir.h"
#include "get_local_ip.h"
#include "findmdahost.h"
#include "UserInLookup.h"
#include "RelayInLookup.h"
#include "AliasInLookup.h"
#include "PwdInLookup.h"
#include "VlimitInLookup.h"
#include "findhost.h"
#include "is_user_present.h"
#include "get_real_domain.h"
#include "sql_get_realdomain.h"
#include "sql_getpw.h"
#include "cntrl_clearaddflag.h"
#include "cntrl_cleardelflag.h"
#include "is_distributed_domain.h"
#include "get_assign.h"
#include "dbload.h"
#include "FifoCreate.h"

#ifndef	lint
static char     sccsid[] = "$Id: ProcessInFifo.c,v 1.14 2022-08-04 14:41:01+05:30 Cprogrammer Exp mbhangui $";
#endif

int             user_query_count, relay_query_count, pwd_query_count, alias_query_count;
int             limit_query_count, dom_query_count, btree_count = 0, _debug;
time_t          start_time;
#ifdef CLUSTERED_SITE
int             host_query_count;
#endif
static void    *in_root = 0;
#if defined(FREEBSD) || defined(DARWIN)
static void    *element = 0;
#endif
char            strnum[FMT_ULONG];
static int      pwdCache; /*- for sighup to figure out if caching was selected on startup */
char           *tcpserver;
void            (*logfunc) ();

/*-
typedef struct
{
	char           *in_key;
	struct passwd   in_pw;
	char           *aliases;
	char           *mdahost;
	char           *domain;
	 *
	 *  0: User is fine
	 *  1: User is not present
	 *  2: User is Inactive
	 *  3: User is overquota
	 * -1: System Error
	 *
	int             in_userStatus;
} INENTRY;
*/

#if defined(FREEBSD) || defined(DARWIN)
/*
 *  This comparison routine can be used with tdelete()
 *  when explicitly deleting a root node, as no comparison
 *  is necessary.
 */
static int
delete_root(const void *node1, const void *node2)
{
    return 0;
}
#endif

static void
die_nomem()
{
	strerr_warn1("inlookup: out of memory", 0);
	_exit(111);
}

static char *
getFifo_name()
{
	static stralloc inFifo = {0};
	char           *infifo_dir, *infifo;

	if ((tcpserver = env_get("TCPREMOTEIP")))
		return ((char *) NULL);
	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&inFifo, infifo) || !stralloc_0(&inFifo))
			die_nomem();
	} else {
		getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
		if (*infifo_dir == '/') {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
			if (!stralloc_copys(&inFifo, infifo_dir) ||
					!stralloc_append(&inFifo, "/") ||
					!stralloc_cats(&inFifo, infifo) ||
					!stralloc_0(&inFifo))
				die_nomem();
		} else {
			if (!stralloc_copys(&inFifo, INDIMAILDIR) ||
					!stralloc_cats(&inFifo, infifo_dir) ||
					!stralloc_append(&inFifo, "/") ||
					!stralloc_cats(&inFifo, infifo) ||
					!stralloc_0(&inFifo))
				die_nomem();
		}
	}
	return (inFifo.s);
}

void
walk_entry(const void *in_data, VISIT x, int level)
{
	INENTRY        *m = *(INENTRY **) in_data;

	strnum[fmt_uint(strnum, level)] = 0;
	logfunc("ProcessInFifo", "<");
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ">Walk on node ");
	logfunc("ProcessInFifo", x == preorder ? "preorder" : x == postorder ? "postorder" : x == endorder ? "endorder" : x == leaf ? "leaf" : "unknown");
	logfunc("ProcessInFifo", " ");
	logfunc("ProcessInFifo", m->in_key);
	logfunc("ProcessInFifo", " ");
	logfunc("ProcessInFifo", m->in_pw.pw_passwd);
	logfunc("ProcessInFifo", "\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	return;
}

char *
in_strdup(char *s)
{
	int             i;
	char           *p;

	i = str_len(s) + 1;
	if (!(p = (char *) alloc(sizeof(char) * i)))
		die_nomem();
	if (str_copyb(p, s, i) != (i - 1)) {
		strerr_warn1("ProcessInFifo: failed to copy data", 0);
		alloc_free(p);
		return ((char *) 0);
	}
	return (p);
}

INENTRY        *
mk_in_entry(char *key)
{
	INENTRY        *in;

	if (!(in = (INENTRY *) alloc(sizeof (INENTRY))))
		return ((INENTRY *) 0);
	if (!(in->in_key = in_strdup(key)))
		return ((INENTRY *) 0);
	in->aliases = in->mdahost = in->domain = (char *) 0;
	in->in_userStatus = -1;
	in->pwStat = 0;
	in->in_pw.pw_name = (char *) 0;
	in->in_pw.pw_passwd = (char *) 0;
	in->in_pw.pw_gecos = (char *) 0;
	in->in_pw.pw_dir = (char *) 0;
	in->in_pw.pw_shell = (char *) 0;
	return in;
}

void
in_free_func(void *in_data)
{
	INENTRY        *m = in_data;
	if (!m)
		return;
	if (m->in_key) {
		alloc_free(m->in_key);
		alloc_free(m->aliases);
		alloc_free(m->mdahost);
		alloc_free(m->domain);
		alloc_free(m->in_pw.pw_name);
		alloc_free(m->in_pw.pw_passwd);
		alloc_free(m->in_pw.pw_gecos);
		alloc_free(m->in_pw.pw_dir);
		alloc_free(m->in_pw.pw_shell);
		m->in_key = 0;
	}
	alloc_free(in_data);
	return;
}

static void
getTimeoutValues(int *readTimeout, int *writeTimeout, char *sysconfdir, char *controldir)
{
	static stralloc tmpbuf = {0}, line = {0};
	char            inbuf[512];
	int             fd, match;
	substdio        ssin;

	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/timeoutread", 12) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/timeoutread", 12) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if ((fd = open_read(tmpbuf.s)) == -1)
		*readTimeout = 4;
	else {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1)
			*readTimeout = 4;
		else {
			if (match) {
				line.len--;
				line.s[line.len] = 0; /*- remove newline */
			}
			scan_uint(line.s, (unsigned int *) readTimeout);
		}
		close(fd);
	}
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/timeoutwrite", 13) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/timeoutwrite", 13) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if ((fd = open_read(tmpbuf.s)) == -1)
		*writeTimeout = 4;
	else {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1)
			*writeTimeout = 4;
		else {
			if (match) {
				line.len--;
				line.s[line.len] = 0; /*- null terminate */
			}
			scan_uint(line.s, (unsigned int *) writeTimeout);
		}
		close(fd);
	}
	return;
}

int
in_compare_func(const void *l, const void *r)
{
	return str_diff(*(char **) l, *(char **) r);
}

int
cache_active_pwd(time_t tval)
{
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	static stralloc SqlBuf = {0}, email = {0};
	int             use_btree, max_btree_count, err, i, len1, len2;
	static time_t   act_secs;
	char           *ptr;
	INENTRY        *in, *re, *retval;

	if (tval)
		act_secs = tval;
	pwdCache = 1;
	ptr = env_get("USE_BTREE");
	use_btree = ((ptr && *ptr == '1') ? 1 : 0);
	if (!use_btree)
		return (0);
	scan_uint((ptr = env_get("MAX_BTREE_COUNT")) && *ptr ? ptr : "-1", (unsigned int *) &max_btree_count);
	if (in_root) {
#if defined(FREEBSD) || defined(DARWIN)
		while (in_root != NULL) {
			element = *(INENTRY **) in_root;
			tdelete(element, &in_root, delete_root);
			in_free_func(element);
		}
#else
		tdestroy(in_root, in_free_func);
#endif
		in_root = 0;
		btree_count = 0;
	}
	if (!stralloc_copyb(&SqlBuf, "SELECT pw_name, pw_domain, pw_passwd, scram, pw_uid, pw_gid, ", 61) ||
			!stralloc_catb(&SqlBuf, "pw_gecos, pw_dir, pw_shell FROM indimail ", 41) ||
			!stralloc_catb(&SqlBuf, "JOIN lastauth ON pw_name = user AND pw_domain = domain WHERE ", 61) ||
			!stralloc_catb(&SqlBuf, "UNIX_timestamp(lastauth.timestamp) >= UNIX_timestamp() - ", 57) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, act_secs)) ||
			!stralloc_catb(&SqlBuf, " AND service in (\"imap\", \"pop3\", \"webm\") ", 41) ||
			!stralloc_catb(&SqlBuf, "GROUP BY pw_name, pw_domain ORDER BY lastauth.timestamp desc", 60) ||
			!stralloc_0(&SqlBuf))
		die_nomem();

	if ((err = iopen((char *) 0)) != 0)
		return (-1);
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
			iclose();
			return (0);
		}
		strerr_warn4("ProcesInFifo: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!(res = in_mysql_store_result(&mysql[1]))) {
		strerr_warn2("ProcesInFifo: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	for(; (row = in_mysql_fetch_row(res));) {
		if (!stralloc_copys(&email, row[0]) ||
				!stralloc_append(&email, "@") ||
				!stralloc_cats(&email, row[1]) ||
				!stralloc_0(&email))
			die_nomem();
		if (!(in = mk_in_entry(email.s))) {
			in_mysql_free_result(res);
			return (-1);
		}
		if (!(retval = tsearch (in, &in_root, in_compare_func))) {
			in_free_func(in);
			in_mysql_free_result(res);
			return (-1);
		} else {
			re = *(INENTRY **) retval;
			if (re != in) { /*- existing data, shouldn't happen */
				in_free_func(in);
				in_mysql_free_result(res);
				return (1);
			} else {/*- New entry in was added.  */
				in->in_pw.pw_name = in_strdup(row[0]);
				in->domain = in_strdup(row[1]);
				if (row[3]) {
					i = (len2 = str_len(row[3])) + (len1 = str_len(row[2])) + 2;
					if (!(ptr = (char *) alloc(sizeof(char) * i)))
						die_nomem();
					str_copyb(ptr, row[3], len2);
					str_copyb(ptr + len2, ",", 1);
					str_copyb(ptr + len2 + 1, row[2], len1);
					ptr[len1 + len2 + 1] = 0;
					in->in_pw.pw_passwd = ptr; /*- scram,pw_passwd */
				} else
					in->in_pw.pw_passwd = in_strdup(row[2]); /*- pw_passwd */
				scan_uint(row[4], &in->in_pw.pw_uid);
				scan_uint(row[5], &in->in_pw.pw_gid);
				in->in_pw.pw_gecos = in_strdup(row[6]);
				in->in_pw.pw_dir = in_strdup(row[7]);
				in->in_pw.pw_shell = in_strdup(row[8]);
				in->pwStat = 1;
				btree_count++;
				if (max_btree_count > 0 && btree_count >= max_btree_count)
					break;
			}
		}
	}
	in_mysql_free_result(res);
	iclose();
	return (0);
}

static char    *
query_type(int status)
{
	static char     tmpbuf[FMT_ULONG + 12];
	char           *s;

	switch(status)
	{
		case USER_QUERY:
			return ("'User Query'");
		case RELAY_QUERY:
			return ("'Relay Query'");
		case PWD_QUERY:
			return ("'Password Query'");
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
			return ("'Host Query'");
#endif
		case ALIAS_QUERY:
			return ("'Alias Query'");
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
			return ("'Domain Limits Query'");
#endif
		case DOMAIN_QUERY:
			return ("'Domain Query'");
		default:
			break;
	}
	s = tmpbuf;
	s += fmt_strn(s, "'Unknown ", 9);
	s += fmt_uint(s, status);
	s += fmt_strn(s, "'\n", 2);
	*s++ = 0;
	return (tmpbuf);
}


#ifdef DARWIN
static void
isig_usr1()
{
	char           *fifo_path;
	long            total_count;
	time_t          cur_time;

	cur_time = time(0);
	fifo_path = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " INFIFO ");
	logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
	logfunc("ProcessInFifo", ", Got SIGUSR Dumping Stats\n");

	logfunc("ProcessInFifo", "User Query ");
	strnum[fmt_uint(strnum, user_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Relay Query ");
	strnum[fmt_uint(strnum, relay_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Password Query ");
	strnum[fmt_uint(strnum, pwd_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ":");
	strnum[fmt_uint(strnum, limit_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Alias Query ");
	strnum[fmt_uint(strnum, alias_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
	logfunc("ProcessInFifo", ", Host Query ");
	strnum[fmt_uint(strnum, host_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	total_count += host_query_count;
#endif
	logfunc("ProcessInFifo", ", Domain Query ");
	strnum[fmt_uint(strnum, dom_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);

	logfunc("ProcessInFifo", " Cached Nodes ");
	strnum[fmt_uint(strnum, btree_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", "\n");

	logfunc("ProcessInFifo", "Start Time: ");
	logfunc("ProcessInFifo", ctime(&start_time));
	logfunc("ProcessInFifo", "End   Time: ");
	logfunc("ProcessInFifo", ctime(&cur_time));
	logfunc("ProcessInFifo", "Queries ");
	strnum[fmt_ulong(strnum, total_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Total Time ");
	strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " secs, Query/Sec = ");
	strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
	logfunc("ProcessInFifo", "\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	twalk(in_root, walk_entry);
	signal(SIGUSR1, (void(*)()) isig_usr1);
	errno = error_intr;
	return;
}

static void
isig_usr2()
{
	char           *fifo_path;

	fifo_path = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " INFIFO ");
	logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
	logfunc("ProcessInFifo", ", Got SIGUSR2 Resetting DEBUG flag to ");
	logfunc("ProcessInFifo", _debug ? "0\n" : "1\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	_debug = (_debug ? 0 : 1);
	signal(SIGUSR2, (void(*)()) isig_usr2);
	errno = error_intr;
	return;
}

static void
isig_hup()
{
	char           *fifo_path;

	signal(SIGHUP, (void(*)()) SIG_IGN);
	fifo_path = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " INFIFO ");
	logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
	logfunc("ProcessInFifo", ", Got SIGHUP Reconfiguring\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
#ifdef QUERY_CACHE
	findhost_cache(0);
	is_user_present_cache(0);
	sql_getpw_cache(0);
	cntrl_clearaddflag_cache(0);
	cntrl_cleardelflag_cache(0);
	is_distributed_domain_cache(0);
	sql_get_realdomain_cache(0);
	get_real_domain_cache(0);
	get_assign_cache(0);
#endif
#if defined(FREEBSD) || defined(DARWIN)
	while (in_root != NULL) {
		element = *(INENTRY **) in_root;
		tdelete(element, &in_root, delete_root);
		in_free_func(element);
	}
#else
	tdestroy(in_root, in_free_func);
#endif
	in_root = 0;
	btree_count = 0;
	if (pwdCache) {
		cache_active_pwd(0);
		strnum[fmt_uint(strnum, btree_count)] = 0;
		logfunc("ProcessInFifo", "cached ");
		logfunc("ProcessInFifo", strnum);
		logfunc("ProcessInFifo", " records\n");
	}
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	signal(SIGHUP, (void(*)()) isig_hup);
	errno = error_intr;
	return;
}

static void
isig_int()
{
	char           *fifo_path;

	fifo_path = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " INFIFO ");
	logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
	logfunc("ProcessInFifo", ", Got SIGINT closing db\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	close_db();
	signal(SIGINT, (void(*)()) isig_int);
	errno = error_intr;
	return;
}

static void
isig_term()
{
	char           *fifo_path;
	long            total_count;
	time_t          cur_time;

	sig_block(SIGTERM);
	cur_time = time(0);
	fifo_path = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (verbose || _debug) {
		logfunc("ProcessInFifo", strnum);
		logfunc("ProcessInFifo", " INFIFO ");
		logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
		logfunc("ProcessInFifo", " ARGH!! Committing suicide on SIGTERM\n");
		(tcpserver ? errflush : flush) ("ProcessInFifo");
	}
	logfunc("ProcessInFifo", "User Query ");
	strnum[fmt_uint(strnum, user_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Relay Query ");
	strnum[fmt_uint(strnum, relay_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Password Query ");
	strnum[fmt_uint(strnum, pwd_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ":");
	strnum[fmt_uint(strnum, limit_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Alias Query ");
	strnum[fmt_uint(strnum, alias_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
	logfunc("ProcessInFifo", ", Host Query ");
	strnum[fmt_uint(strnum, host_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	total_count += host_query_count;
#endif
	logfunc("ProcessInFifo", ", Domain Query ");
	strnum[fmt_uint(strnum, dom_query_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " Cached Nodes ");
	strnum[fmt_uint(strnum, btree_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", "\n");

	logfunc("ProcessInFifo", "Start Time: ");
	logfunc("ProcessInFifo", ctime(&start_time));
	logfunc("ProcessInFifo", "End   Time: ");
	logfunc("ProcessInFifo", ctime(&cur_time));
	logfunc("ProcessInFifo", "Queries ");
	strnum[fmt_ulong(strnum, total_count)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", ", Total Time ");
	strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " secs, Query/Sec = ");
	strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
	logfunc("ProcessInFifo", "\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	close_db();
	if (fifo_path)
		unlink(fifo_path);
	_exit(0);
}
#else
static void
sig_hand(sig, code, scp, addr)
	int             sig, code;
	struct sigcontext *scp;
	char           *addr;
{
	char           *fifo_path;
	long            total_count;
	time_t          cur_time;

	fifo_path = getFifo_name();
	if (sig == SIGTERM) {
		sig_block(sig);
		if (verbose || _debug) {
			strnum[fmt_ulong(strnum, getpid())] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " INFIFO ");
			logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
			logfunc("ProcessInFifo", " ARGH!! Committing suicide on SIGTERM\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
		}
	}
	if (sig != SIGTERM || verbose || _debug) {
		strnum[fmt_ulong(strnum, getpid())] = 0;
		logfunc("ProcessInFifo", strnum);
		logfunc("ProcessInFifo", " INFIFO ");
		logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
		logfunc("ProcessInFifo", ", Got ");
#ifdef HAVE_STRSIGNAL
		logfunc("ProcessInFifo", (char *) strsignal(sig));
#else
		logfunc("ProcessInFifo", (char *) sys_siglist[sig]);
#endif
		logfunc("ProcessInFifo", "\n");
		(tcpserver ? errflush : flush) ("ProcessInFifo");
	}
	switch (sig)
	{
		case SIGUSR1:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " INFIFO ");
			logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
			logfunc("ProcessInFifo", ", Dumping stats\n");
			/*- flow through */
		case SIGTERM:
			cur_time = time(0);
			logfunc("ProcessInFifo", "User Query ");
			strnum[fmt_uint(strnum, user_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ", Relay Query ");
			strnum[fmt_uint(strnum, relay_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ", Password Query ");
			strnum[fmt_uint(strnum, pwd_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ":");
			strnum[fmt_uint(strnum, limit_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ", Alias Query ");
			strnum[fmt_uint(strnum, alias_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
			logfunc("ProcessInFifo", ", Host Query ");
			strnum[fmt_uint(strnum, host_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			total_count += host_query_count;
#endif
			logfunc("ProcessInFifo", ", Domain Query ");
			strnum[fmt_uint(strnum, dom_query_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " Cached Nodes ");
			strnum[fmt_uint(strnum, btree_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", "\n");
			logfunc("ProcessInFifo", "Start Time: ");
			logfunc("ProcessInFifo", ctime(&start_time));
			logfunc("ProcessInFifo", "End   Time: ");
			logfunc("ProcessInFifo", ctime(&cur_time));
			logfunc("ProcessInFifo", "Queries ");
			strnum[fmt_ulong(strnum, total_count)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ", Total Time ");
			strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " secs, Query/Sec = ");
			strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
			logfunc("ProcessInFifo", "\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
			if (sig == SIGUSR1)
				twalk(in_root, walk_entry);
			if (sig == SIGTERM) {
				(tcpserver ? errflush : flush) ("ProcessInFifo");
				close_db();
				if (fifo_path)
					unlink(fifo_path);
				_exit(0);
			}
			break;
		case SIGUSR2:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " INFIFO ");
			logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
			logfunc("ProcessInFifo", " Resetting DEBUG flag to ");
			logfunc("ProcessInFifo", _debug ? "0\n" : "1\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
			_debug = (_debug ? 0 : 1);
			break;
		case SIGHUP:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " INFIFO ");
			logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
			logfunc("ProcessInFifo", ", Reconfiguring\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
			close_db();
#ifdef QUERY_CACHE
			findhost_cache(0);
			is_user_present_cache(0);
			sql_getpw_cache(0);
			cntrl_clearaddflag_cache(0);
			cntrl_cleardelflag_cache(0);
			is_distributed_domain_cache(0);
			sql_get_realdomain_cache(0);
			get_assign_cache(0);
			get_real_domain_cache(0);
#endif
#if defined(FREEBSD) || defined(DARWIN)
			while (in_root != NULL) {
				element = *(INENTRY **) in_root;
				tdelete(element, &in_root, delete_root);
				in_free_func(element);
			}
#else
			tdestroy(in_root, in_free_func);
#endif
			in_root = 0;
			btree_count = 0;
		break;
		case SIGINT:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " INFIFO ");
			logfunc("ProcessInFifo", fifo_path ? fifo_path : "socket");
			logfunc("ProcessInFifo", " closing db\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
			close_db();
		break;
	} /*- switch (sig) */
	signal(sig, (void(*)()) sig_hand);
	errno = error_intr;
	return;
}
#endif /*- #ifdef DARWIN */

int
ProcessInFifo(int instNum)
{
	int             i, rfd, wfd, bytes, status, idx, pipe_size, readTimeout,
					writeTimeout, use_btree, max_btree_count, fd, match;
	INENTRY        *in, *re, *retval;
	struct passwd  *pw;
	static stralloc pwbuf = {0}, host_path = {0}, line = {0};
	char            tmp[FMT_ULONG], inbuf[512];
	char           *ptr, *fifoName, *fifo_path, *myFifo, *sysconfdir, *controldir,
				   *QueryBuf, *email, *remoteip, *local_ip, *cntrl_host,
				   *real_domain;
	void            (*pstat) () = NULL;
	void           *(*search_func) (const void *key, void *const *rootp, int (*compar)(const void *, const void *));
	time_t          prev_time = 0l;
	substdio        ssin;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

	_debug = (env_get("DEBUG") ? 1 : 0);
	start_time = time(0);
	tcpserver = env_get("TCPREMOTEIP");
	if (!tcpserver) {
#ifdef DARWIN
		signal(SIGTERM, (void(*)()) isig_term);
		signal(SIGUSR1, (void(*)()) isig_usr1);
		signal(SIGUSR2, (void(*)()) isig_usr2);
		signal(SIGHUP, (void(*)()) isig_hup);
		signal(SIGINT, (void(*)()) isig_int);
#else
		signal(SIGTERM, (void(*)()) sig_hand);
		signal(SIGUSR1, (void(*)()) sig_hand);
		signal(SIGUSR2, (void(*)()) sig_hand);
		signal(SIGHUP, (void(*)()) sig_hand);
		signal(SIGINT, (void(*)()) sig_hand);
#endif
	}
	logfunc = tcpserver ? errout : out;
	logfunc("ProcessInFifo", "InLookup[");
	if (!tcpserver) {
		strnum[fmt_uint(strnum, instNum)] = 0;
		logfunc("ProcessInFifo", strnum);
	} else {
		logfunc("ProcessInFifo", tcpserver);
	}
	logfunc("ProcessInFifo", "] PPID ");
	strnum[fmt_uint(strnum, getppid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", " PID ");
	strnum[fmt_uint(strnum, getpid())] = 0;
	logfunc("ProcessInFifo", strnum);
	logfunc("ProcessInFifo", "\n");
	(tcpserver ? errflush : flush) ("ProcessInFifo");
	fifo_path = getFifo_name();
	if (!tcpserver) {
		/*- Open the Fifos */
		if (FifoCreate(fifo_path) == -1) {
			strerr_warn3("InLookup: FifoCreate: ", fifo_path, ": ", &strerr_sys);
			return (-1);
		} else
		if ((rfd = open(fifo_path, O_RDWR)) == -1) {
			strerr_warn3("InLookup: open O_RDWR: ", fifo_path, ": ", &strerr_sys);
			return (-1);
		} else
		if ((pipe_size = fpathconf(rfd, _PC_PIPE_BUF)) == -1) {
			strerr_warn3("InLookup: fpathconf _PC_PIPE_BUF: ", fifo_path, ": ", &strerr_sys);
			return (-1);
		}
		if (!(QueryBuf = (char *) alloc(pipe_size * sizeof(char))))
			die_nomem();
	} else {
		pipe_size = 1024;
		if (!(QueryBuf = (char *) alloc(pipe_size * sizeof(char))))
			die_nomem();
		rfd = 0;
	}
	user_query_count = relay_query_count = pwd_query_count = limit_query_count = alias_query_count = dom_query_count = 0;
#ifdef CLUSTERED_SITE
	host_query_count = 0;
#endif
	if (!(local_ip = get_local_ip(PF_INET))) {
		local_ip = "127.0.0.1";
		strerr_warn1("ProcessInFifo: get_local_ip failed. using localhost", 0);
	}
	if (!tcpserver) {
		if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
			strerr_warn1("InLookup: signal: ", &strerr_sys);
			return (-1);
		}
		ptr = env_get("USE_BTREE");
		use_btree = ((ptr && *ptr == '1') ? 1 : 0);
		scan_uint((ptr = env_get("MAX_BTREE_COUNT")) && *ptr ? ptr : "-1", (unsigned int *) &max_btree_count);
		search_func = (void *) tsearch; /*- this adds a record if not found */
		fifoName = fifo_path;
		match = str_rchr(fifo_path, '/');
		if (fifo_path[match])
			fifoName = fifo_path + match + 1;
	} else {
		use_btree = 0;
		fifoName = fifo_path = tcpserver;
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getTimeoutValues(&readTimeout, &writeTimeout, sysconfdir, controldir);
	for (bytes = 0;getppid() != 1;) {
		/*- read size of query buffer */
		if ((idx = read(rfd, (char *) &bytes, sizeof(int))) == -1) {
#ifdef ERESTART
			if (errno != error_intr && errno != error_restart)
#else
			if (errno != error_intr)
#endif
			{
				strerr_warn1("InLookup: read: ", &strerr_sys);
				logfunc("ProcessInFifo", "read: ");
				logfunc("ProcessInFifo", error_str(errno));
				logfunc("ProcessInFifo", "\n");
				(tcpserver ? errflush : flush) ("ProcessInFifo");
				sleep(1);
			}
			continue;
		} else
		if (!idx) {
			close(rfd);
			if (!tcpserver) {
				if ((rfd = open(fifo_path, O_RDWR)) == -1) {
					strerr_warn3("InLookup: reopen O_RDWR: ", fifo_path, ": ", &strerr_sys);
					signal(SIGPIPE, pstat);
					return (-1);
				} else {
					strerr_warn1("InLookup: aborted read from client", 0);
					continue;
				}
			} else
				return (0);
		} else
		if (bytes > pipe_size) {
			errno = EMSGSIZE;
			strnum[fmt_uint(strnum, bytes)] = 0;
			tmp[fmt_uint(tmp, pipe_size)] = 0;
			strerr_warn5("InLookup: bytes ", strnum, ", pipe_size ", tmp, ": ", &strerr_sys);
			if (tcpserver)
				return (1);
			continue;
		} else /* another read to fetch query buffer */
		if ((idx = timeoutread(readTimeout, rfd, QueryBuf, bytes)) == -1) {
			strerr_warn1("InLookup: read-int: ", &strerr_sys);
			if (tcpserver)
				return (1);
			continue;
		} else
		if (!idx) {
			if (!tcpserver) {
				close(rfd);
				if ((rfd = open(fifo_path, O_RDWR)) == -1) {
					strerr_warn3("InLookup: reopen O_RDWR: ", fifo_path, ": ", &strerr_sys);
					signal(SIGPIPE, pstat);
					return (-1);
				} else {
					strerr_warn1("InLookup: aborted read from client", 0);
					continue;
				}
			} else
				return (-1); /*- partial read */
		}
		if (verbose || _debug)
			prev_time = time(0);
#ifdef CLUSTERED_SITE
		if (*controldir == '/') {
			if (!stralloc_copys(&host_path, controldir) ||
					!stralloc_catb(&host_path, "/host.cntrl", 11) ||
					!stralloc_0(&host_path))
				die_nomem();
		} else {
			if (!stralloc_copys(&host_path, sysconfdir) ||
					!stralloc_append(&host_path, "/") ||
					!stralloc_cats(&host_path, controldir) ||
					!stralloc_catb(&host_path, "/host.cntrl", 11) ||
					!stralloc_0(&host_path))
				die_nomem();
		}
		if ((fd = open_read(host_path.s)) == -1) {
			if (errno != error_noent) {
				strerr_warn3("InLookup: open: ", host_path.s, ": ", &strerr_sys);
				return (-1);
			}
			cntrl_host = 0;
			line.len = 0;
		} else {
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("InLookup: read: ", host_path.s, ": ", &strerr_sys);
				close(fd);
				return (-1);
			}
			if (match) {
				line.len--;
				line.s[line.len] = 0;
			}
			cntrl_host = line.s;
			close(fd);
		}
		if (line.len) {
			if (!isopen_cntrl && open_central_db(cntrl_host)) {
				strerr_warn1("InLookup: Unable to open central db", 0);
				if (!tcpserver)
					signal(SIGPIPE, pstat);
				return (-1);
			}
			if (in_mysql_ping(&mysql[0])) {
				strerr_warn3("mysql_ping: ", (char *) in_mysql_error(&mysql[0]) ,": Reconnecting to central db...", 0);
				in_mysql_close(&mysql[0]);
				isopen_cntrl = 0;
				if (open_central_db(cntrl_host)) {
					strerr_warn1("InLookup: Unable to open central db", 0);
					if (!tcpserver)
						signal(SIGPIPE, pstat);
					return (-1);
				}
			}
		}
#endif
		switch(*QueryBuf)
		{
			case USER_QUERY:
				user_query_count++;
				break;
			case RELAY_QUERY:
				relay_query_count++;
				break;
			case PWD_QUERY:
				pwd_query_count++;
				break;
#ifdef CLUSTERED_SITE
			case HOST_QUERY:
				host_query_count++;
				break;
#endif
			case ALIAS_QUERY:
				alias_query_count++;
				break;
#ifdef ENABLE_DOMAIN_LIMITS
			case LIMIT_QUERY:
				limit_query_count++;
				break;
#endif
			case DOMAIN_QUERY:
				dom_query_count++;
				break;
			default:
				continue;
		}
		email = QueryBuf + 2;
		for (ptr = email; *ptr; ptr++);
		ptr++;
		myFifo = ptr;
		for (; *ptr; ptr++);
		ptr++;
		remoteip = ptr;
		if (verbose || _debug) {
			logfunc("ProcessInFifo", fifoName);
			logfunc("ProcessInFifo", "->");
			logfunc("ProcessInFifo", myFifo);
			logfunc("ProcessInFifo", ", Bytes ");
			strnum[fmt_int(strnum, bytes)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", ", Query [");
			logfunc("ProcessInFifo", query_type(*QueryBuf));
			logfunc("ProcessInFifo", "], User ");
			logfunc("ProcessInFifo", email);
			logfunc("ProcessInFifo", ", RemoteIp ");
			logfunc("ProcessInFifo", *QueryBuf == 2 ? remoteip : "N/A");
			logfunc("ProcessInFifo", "\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
		}
		if (!tcpserver) {
			if ((wfd = open_write(myFifo)) == -1) {
				strerr_warn5(errno == error_acces ?  "InLookup: open-write: " : "InLookup: open-probably-timeout: ",
						myFifo, ": QueryType ", query_type(*QueryBuf), ": ", &strerr_sys);
				if (errno != ENOENT && unlink(myFifo))
					strerr_warn3("InLookup: unlink: ", myFifo, ": ", &strerr_sys);
				continue;
			} else
			if (unlink(myFifo)) /*- make this invisible */
				strerr_warn3("InLookup: unlink: ", myFifo, ": ", &strerr_sys);
		} else
			wfd = 1;
		switch(*QueryBuf)
		{
			case USER_QUERY:
				if (!use_btree || !(in = mk_in_entry(email)))
					status = UserInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					status = UserInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						status = re->in_userStatus;
						if (status < 0) {
							status = UserInLookup(email);
							re->in_userStatus = status;
						} else
						if (verbose || _debug) {
							logfunc("ProcessInFifo", fifoName);
							logfunc("ProcessInFifo", "->");
							logfunc("ProcessInFifo", myFifo);
							logfunc("ProcessInFifo", ", cache hit\n");
							(tcpserver ? errflush : flush) ("ProcessInFifo");
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						status = UserInLookup(email);
						in->in_userStatus = status;
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-UserInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
			case RELAY_QUERY:
				status = RelayInLookup(email, remoteip);
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-RelayInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
#ifdef CLUSTERED_SITE
			case HOST_QUERY:
				if (!use_btree || !(in = mk_in_entry(email)))
					ptr = findmdahost(email, 0);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					ptr = findmdahost(email, 0);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						ptr = re->mdahost;
						if (!ptr) {
							if ((ptr = findmdahost(email, 0)))
								re->mdahost = in_strdup(ptr);
						} else
						if (verbose || _debug) {
							logfunc("ProcessInFifo", fifoName);
							logfunc("ProcessInFifo", "->");
							logfunc("ProcessInFifo", myFifo);
							logfunc("ProcessInFifo", ", cache hit\n");
							(tcpserver ? errflush : flush) ("ProcessInFifo");
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((ptr = findmdahost(email, 0)))
							in->mdahost = in_strdup(ptr);
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (ptr)
					bytes = str_len(ptr) + 1;
				else
					bytes = (userNotFound ? 0 : -1);
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-findmdahost: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				} else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1) {
					strerr_warn1("InLookup: write-findmdahost: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
#endif
			case ALIAS_QUERY:
				if (!use_btree || !(in = mk_in_entry(email)))
					ptr = AliasInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					ptr = AliasInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						ptr = re->aliases;
						if (!ptr) {
							if ((ptr = AliasInLookup(email)))
								re->aliases = in_strdup(ptr);
						} else
						if (verbose || _debug) {
							logfunc("ProcessInFifo", fifoName);
							logfunc("ProcessInFifo", "->");
							logfunc("ProcessInFifo", myFifo);
							logfunc("ProcessInFifo", ", cache hit\n");
							(tcpserver ? errflush : flush) ("ProcessInFifo");
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((ptr = AliasInLookup(email)))
							in->aliases = in_strdup(ptr);
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (ptr && *ptr) {
					if ((bytes = str_len(ptr) + 1) > pipe_size)
						bytes = -1;
				} else {
					bytes = 1; /*- write Null Byte */
					ptr = "\0";
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-AliasInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				} else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1) {
					strerr_warn1("InLookup: write-AliasInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
			case PWD_QUERY:
				i = str_rchr(email, '@');
				if (!email[i])
					getEnvConfigStr(&real_domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
				else
					real_domain = email + i + 1;
				if (!use_btree || !(in = mk_in_entry(email)))
					pw = PwdInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					pw = PwdInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						if (!re->pwStat) { /*- incomplete existing data */
							if ((pw = PwdInLookup(email))) {
								re->in_pw.pw_uid = pw->pw_uid;
								re->in_pw.pw_gid = pw->pw_gid;
								re->in_pw.pw_name = in_strdup(pw->pw_name);
								re->domain = in_strdup(real_domain);
								re->in_pw.pw_passwd = in_strdup(pw->pw_passwd);
								re->in_pw.pw_gecos = in_strdup(pw->pw_gecos);
								re->in_pw.pw_dir = in_strdup(pw->pw_dir);
								re->in_pw.pw_shell = in_strdup(pw->pw_shell);
								re->pwStat = 1;
							}
						} else { /*- completed existing data */
							pw = &re->in_pw;
							real_domain = re->domain;
							if (verbose || _debug) {
								logfunc("ProcessInFifo", fifoName);
								logfunc("ProcessInFifo", "->");
								logfunc("ProcessInFifo", myFifo);
								logfunc("ProcessInFifo", ", cache hit\n");
								(tcpserver ? errflush : flush) ("ProcessInFifo");
							}
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((pw = PwdInLookup(email))) {
							in->in_pw.pw_uid = pw->pw_uid;
							in->in_pw.pw_gid = pw->pw_gid;
							re->in_pw.pw_name = in_strdup(pw->pw_name);
							re->domain = in_strdup(real_domain);
							re->in_pw.pw_passwd = in_strdup(pw->pw_passwd);
							re->in_pw.pw_gecos = in_strdup(pw->pw_gecos);
							re->in_pw.pw_dir = in_strdup(pw->pw_dir);
							re->in_pw.pw_shell = in_strdup(pw->pw_shell);
							in->pwStat = 1;
						} else
							in->pwStat = 0;
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (pw) {
					if (!stralloc_copyb(&pwbuf, "PWSTRUCT=", 9) ||
							!stralloc_cats(&pwbuf, pw->pw_name) ||
							!stralloc_append(&pwbuf, "@") ||
							!stralloc_cats(&pwbuf, real_domain) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_cats(&pwbuf, pw->pw_passwd) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_catb(&pwbuf, strnum, fmt_uint(strnum, pw->pw_uid)) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_catb(&pwbuf, strnum, fmt_uint(strnum, pw->pw_gid)) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_cats(&pwbuf, pw->pw_gecos) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_cats(&pwbuf, pw->pw_dir) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_cats(&pwbuf, pw->pw_shell) ||
							!stralloc_append(&pwbuf, ":") ||
							!stralloc_catb(&pwbuf, strnum, fmt_int(strnum, is_inactive)) ||
							!stralloc_0(&pwbuf))
						die_nomem();
					if ((bytes = (pwbuf.len)) > pipe_size)
						bytes = -1;
				} else
				if (userNotFound) {
					if (!stralloc_copyb(&pwbuf, "PWSTRUCT=No such user ", 22) ||
							!stralloc_cats(&pwbuf, email) ||
							!stralloc_0(&pwbuf))
						die_nomem();
					bytes = pwbuf.len;
				} else
					bytes = 0;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-PwdInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				} else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, pwbuf.s, bytes) == -1) {
					strerr_warn1("InLookup: write-PwdInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
#ifdef ENABLE_DOMAIN_LIMITS
			case LIMIT_QUERY:
				if ((status = VlimitInLookup(email, &limits)) == -1)
					bytes = -1;
				else
				if (status) /*- user not found */
					bytes = 0;
				else {
					bytes = sizeof(struct vlimits);
					if (bytes > pipe_size)
						bytes = -1;
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-VlimitInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				} else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, (char *) &limits, bytes) == -1) {
					strerr_warn1("InLookup: write-VlimitInLookup: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
#endif
			case DOMAIN_QUERY:
				if (!(real_domain = get_real_domain(email)))
					real_domain = email;
				bytes = str_len(real_domain) + 1;
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1) {
					strerr_warn1("InLookup: write-get_real_domain: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				} else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, real_domain, bytes) == -1) {
					strerr_warn1("InLookup: write-get_real_domain: ", &strerr_sys);
					if (tcpserver)
						return (-1);
				}
				if (!tcpserver)
					close(wfd);
				break;
		} /*- switch(*QueryBuf) */
		if (verbose || _debug) {
			strnum[fmt_ulong(strnum, time(0) - prev_time)] = 0;
			logfunc("ProcessInFifo", strnum);
			logfunc("ProcessInFifo", " ");
			logfunc("ProcessInFifo", query_type(*QueryBuf));
			logfunc("ProcessInFifo", " -> ");
			logfunc("ProcessInFifo", myFifo);
			logfunc("ProcessInFifo", "\n");
			(tcpserver ? errflush : flush) ("ProcessInFifo");
		}
		if (tcpserver)
			return (0);
	} /*- for (QueryBuf = (char *) 0;;) */
	signal(SIGPIPE, pstat);
	return (1);
}
