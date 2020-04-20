/*
 * $Log: ProcessInFifo.c,v $
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
#ifdef ENABLE_ENTERPRISE
#include <dlfcn.h>
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
#ifdef ENABLE_ENTERPRISE
#include "count_table.h"
#endif
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
static char     sccsid[] = "$Id: ProcessInFifo.c,v 1.5 2020-04-01 18:57:29+05:30 Cprogrammer Exp mbhangui $";
#endif

int             user_query_count, relay_query_count, pwd_query_count, alias_query_count;
int             limit_query_count, dom_query_count, btree_count = 0, _debug;
time_t          start_time;
#ifdef CLUSTERED_SITE
int             host_query_count;
#endif
static void    *in_root = 0;
char            strnum[FMT_ULONG];

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

	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&inFifo, infifo) || !stralloc_0(&inFifo))
			die_nomem();
	} else {
		getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
		if (*infifo_dir == '/') {
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
	out("ProcessInFifo", "<");
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ">Walk on node ");
	out("ProcessInFifo", x == preorder ? "preorder" : x == postorder ? "postorder" : x == endorder ? "endorder" : x == leaf ? "leaf" : "unknown");
	out("ProcessInFifo", " ");
	out("ProcessInFifo", m->in_key);
	out("ProcessInFifo", " ");
	out("ProcessInFifo", m->in_pw.pw_passwd);
	out("ProcessInFifo", "\n");
	flush("ProcessInFifo");
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

#ifdef DARWIN
static void
sig_usr1()
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	cur_time = time(0);
	fifo_name = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", ", Got SIGUSR1\n");
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", " Dumping Stats\n");

	out("ProcessInFifo", "User Query ");
	strnum[fmt_uint(strnum, user_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Relay Query ");
	strnum[fmt_uint(strnum, relay_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Password Query ");
	strnum[fmt_uint(strnum, pwd_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ":");
	strnum[fmt_uint(strnum, limit_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Alias Query ");
	strnum[fmt_uint(strnum, alias_query_count)] = 0;
	out("ProcessInFifo", strnum);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
	out("ProcessInFifo", ", Host Query ");
	strnum[fmt_uint(strnum, host_query_count)] = 0;
	out("ProcessInFifo", strnum);
	total_count += host_query_count;
#endif
	out("ProcessInFifo", ", Domain Query ");
	strnum[fmt_uint(strnum, dom_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " Cached Nodes ");
	strnum[fmt_uint(strnum, btree_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", "\n");

	out("ProcessInFifo", "Start Time: ");
	out("ProcessInFifo", ctime(&start_time));
	out("ProcessInFifo", "End   Time: ");
	out("ProcessInFifo", ctime(&cur_time));
	out("ProcessInFifo", "Queries ");
	strnum[fmt_ulong(strnum, total_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Total Time ");
	strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " secs, Query/Sec = ");
	strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
	out("ProcessInFifo", "\n");
	flush("ProcessInFifo");
	twalk(in_root, walk_entry);
	signal(SIGUSR1, (void(*)()) sig_usr1);
	errno = error_intr;
	return;
}

static void
sig_usr2()
{
	char           *fifo_name;

	fifo_name = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", ", Got SIGUSR2\n");

	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", " Resetting DEBUG flag to ");
	out("ProcessInFifo", _debug ? "0\n" : "1\n");
	flush("ProcessInFifo");
	_debug = (_debug ? 0 : 1);
	signal(SIGUSR2, (void(*)()) sig_usr2);
	errno = error_intr;
	return;
}

static void
sig_hup()
{
	char           *fifo_name;

	signal(SIGHUP, (void(*)()) SIG_IGN);
	fifo_name = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", ", Got SIGHUP\n");
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", " Reconfiguring\n");
	flush("ProcessInFifo");
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
	tdestroy(in_root, in_free_func);
	in_root = 0;
	btree_count = 0;
	if (pwdCache) {
		cache_active_pwd(0);
		strnum[fmt_uint(strnum, btree_count)] = 0;
		out("ProcessInFifo", "cached ");
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " records\n");
	}
	flush("ProcessInFifo");
	signal(SIGHUP, (void(*)()) sig_hup);
	errno = error_intr;
	return;
}

static void
sig_int()
{
	char           *fifo_name;

	fifo_name = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", ", Got SIGINT\n");
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " INFIFO ");
	out("ProcessInFifo", fifo_name);
	out("ProcessInFifo", " closing db\n");
	flush("ProcessInFifo");
	close_db();
	signal(SIGINT, (void(*)()) sig_int);
	errno = error_intr;
	return;
}

static void
sig_term()
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	sig_block(SIGTERM);
	cur_time = time(0);
	fifo_name = getFifo_name();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (verbose || _debug) {
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " INFIFO ");
		out("ProcessInFifo", fifo_name);
		out("ProcessInFifo", " ARGH!! Committing suicide on SIGTERM\n");
		flush("ProcessInFifo");
	}
	out("ProcessInFifo", "User Query ");
	strnum[fmt_uint(strnum, user_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Relay Query ");
	strnum[fmt_uint(strnum, relay_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Password Query ");
	strnum[fmt_uint(strnum, pwd_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ":");
	strnum[fmt_uint(strnum, limit_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Alias Query ");
	strnum[fmt_uint(strnum, alias_query_count)] = 0;
	out("ProcessInFifo", strnum);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
	out("ProcessInFifo", ", Host Query ");
	strnum[fmt_uint(strnum, host_query_count)] = 0;
	out("ProcessInFifo", strnum);
	total_count += host_query_count;
#endif
	out("ProcessInFifo", ", Domain Query ");
	strnum[fmt_uint(strnum, dom_query_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " Cached Nodes ");
	strnum[fmt_uint(strnum, btree_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", "\n");

	out("ProcessInFifo", "Start Time: ");
	out("ProcessInFifo", ctime(&start_time));
	out("ProcessInFifo", "End   Time: ");
	out("ProcessInFifo", ctime(&cur_time));
	out("ProcessInFifo", "Queries ");
	strnum[fmt_ulong(strnum, total_count)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", ", Total Time ");
	strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " secs, Query/Sec = ");
	strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
	out("ProcessInFifo", "\n");
	flush("ProcessInFifo");
	close_db();
	unlink(fifo_name);
	_exit(0);
}
#else
static void
sig_hand(sig, code, scp, addr)
	int             sig, code;
	struct sigcontext *scp;
	char           *addr;
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	fifo_name = getFifo_name();
	if (sig == SIGTERM) {
		sig_block(sig);
		if (verbose || _debug) {
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", " ARGH!! Committing suicide on SIGTERM\n");
			flush("ProcessInFifo");
		}
	}
	if (sig != SIGTERM || verbose || _debug) {
		strnum[fmt_ulong(strnum, getpid())] = 0;
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " INFIFO ");
		out("ProcessInFifo", fifo_name);
		out("ProcessInFifo", ", Got ");
		out("ProcessInFifo", (char *) sys_siglist[sig]);
		out("ProcessInFifo", "\n");
		flush("ProcessInFifo");
	}
	switch (sig)
	{
		case SIGUSR1:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", ", Got SIGUSR1\n");
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", " Dumping Stats\n");
			/*- flow through */
		case SIGTERM:
			cur_time = time(0);
			out("ProcessInFifo", "User Query ");
			strnum[fmt_uint(strnum, user_query_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ", Relay Query ");
			strnum[fmt_uint(strnum, relay_query_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ", Password Query ");
			strnum[fmt_uint(strnum, pwd_query_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ":");
			strnum[fmt_uint(strnum, limit_query_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ", Alias Query ");
			strnum[fmt_uint(strnum, alias_query_count)] = 0;
			out("ProcessInFifo", strnum);
			total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#ifdef CLUSTERED_SITE
			out("ProcessInFifo", ", Host Query ");
			strnum[fmt_uint(strnum, host_query_count)] = 0;
			out("ProcessInFifo", strnum);
			total_count += host_query_count;
#endif
			out("ProcessInFifo", ", Domain Query ");
			strnum[fmt_uint(strnum, dom_query_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " Cached Nodes ");
			strnum[fmt_uint(strnum, btree_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", "\n");
			out("ProcessInFifo", "Start Time: ");
			out("ProcessInFifo", ctime(&start_time));
			out("ProcessInFifo", "End   Time: ");
			out("ProcessInFifo", ctime(&cur_time));
			out("ProcessInFifo", "Queries ");
			strnum[fmt_ulong(strnum, total_count)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ", Total Time ");
			strnum[fmt_ulong(strnum, cur_time - start_time)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " secs, Query/Sec = ");
			strnum[fmt_double(strnum, (float) ((float) total_count/(cur_time - start_time)), 2)] = 0;
			out("ProcessInFifo", "\n");
			flush("ProcessInFifo");
			if (sig == SIGUSR1)
				twalk(in_root, walk_entry);
			if (sig == SIGTERM) {
				flush("ProcessInFifo");
				close_db();
				unlink(fifo_name);
				_exit(0);
			}
			break;
		case SIGUSR2:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", " Resetting DEBUG flag to ");
			out("ProcessInFifo", _debug ? "0\n" : "1\n");
			flush("ProcessInFifo");
			_debug = (_debug ? 0 : 1);
			break;
		case SIGHUP:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", ", Got SIGHUP\n");
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", " Reconfiguring\n");
			flush("ProcessInFifo");
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
			tdestroy(in_root, in_free_func);
			in_root = 0;
			btree_count = 0;
		break;
		case SIGINT:
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " INFIFO ");
			out("ProcessInFifo", fifo_name);
			out("ProcessInFifo", " closing db\n");
			flush("ProcessInFifo");
			close_db();
		break;
	} /*- switch (sig) */
	signal(sig, (void(*)()) sig_hand);
	errno = error_intr;
	return;
}
#endif /*- #ifdef DARWIN */

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

static int      pwdCache; /*- for sighup to figure out if caching was selected on startup */

int
cache_active_pwd(time_t tval)
{
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	static stralloc SqlBuf = {0}, email = {0};
	int             use_btree, max_btree_count, err;
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
		tdestroy(in_root, in_free_func);
		in_root = 0;
		btree_count = 0;
	}
	if (!stralloc_copyb(&SqlBuf, "SELECT pw_name, pw_domain, pw_passwd, pw_uid, pw_gid, ", 54) ||
			!stralloc_catb(&SqlBuf, "pw_gecos, pw_dir, pw_shell FROM indimail ", 41) ||
			!stralloc_catb(&SqlBuf, "JOIN lastauth ON pw_name = user AND pw_domain = domain WHERE ", 61) ||
			!stralloc_catb(&SqlBuf, "UNIX_timestamp(lastauth.timestamp) >= UNIX_timestamp() - ", 57) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, act_secs)) ||
			!stralloc_catb(&SqlBuf, " AND service in (\"imap\", \"pop3\", \"wtbm\") ", 41) ||
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
				in->in_pw.pw_passwd = in_strdup(row[2]);
				in->in_pw.pw_gecos = in_strdup(row[5]);
				in->in_pw.pw_dir = in_strdup(row[6]);
				in->in_pw.pw_shell = in_strdup(row[7]);
				scan_uint(row[3], &in->in_pw.pw_uid);
				scan_uint(row[4], &in->in_pw.pw_gid);
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

#ifdef ENABLE_ENTERPRISE
int
do_startup(int instNum)
{
	void           *handle;
	char            tmp[FMT_ULONG];
	char           *plugindir, *plugin_symb, *start_plugin, *error;
	static stralloc plugin = {0};
	int             (*func) (void);
	int             i, status;

	if (!(plugindir = env_get("PLUGINDIR")))
		plugindir = "plugins";
	i = strchr(plugindir, '/');
	if (plugindir[i]) {
		strerrr_warn1("alert: plugindir cannot have an absolute path", 0);
		return (-1);
	}
	if (!(plugin_symb = env_get("START_PLUGIN_SYMB")))
		plugin_symb = "startup";
	if (!(start_plugin = env_get("START_PLUGIN")))
		start_plugin = "indimail-license.so";
	if (!stralloc_copyb(&plugin, "/usr/lib/indimail/", 18) ||
			!stralloc_cats(&plugin, plugindir) ||
			!stralloc_append(&plugin, "/") ||
			!stralloc_cats(&plugin, start_plugin) ||
			!stralloc_0(&plugin))
		die_nomem();
	strnum[fmt_uint(strnum, instNum)] = 0;
	if (access(plugin.s, F_OK)) {
		strerr_warn5("InLookup[", strnum, "] plugin ", plugin.s, ": ", &strerr_sys);
		return (2);
	}
	if (!(handle = dlopen(plugin, RTLD_LAZY|RTLD_GLOBAL))) {
		strerr_warn6("InLookup[", strnum, "] dlopen failed for ", plugin.s, ": ", dlerror(), 0);
		return (-1);
	}
	dlerror(); /*- man page told me to do this */
	func = dlsym(handle, plugin_symb);
	if ((error = dlerror())) {
		strerr_warn6("InLookup[", strnum, "] dlsym ", plugin_symb, " failed: ", error, 0);
		_exit(111);
	}
	out("ProcessInFifo", "InLookup[");
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", "] Checking Plugin ");
	out("ProcessInFifo", start_plugin);
	out("ProcessInFifo", "\n");
	flush("ProcessInFifo");
	if ((status = (*func) ()))
		tmp[fmt_int(tmp, status)] = 0;
		strerr_warn6("InLookup[", strnum, "] function ", plugin_symb, " failed with status ", tmp, 0);
	if (dlclose(handle)) {
		strerr_warn6("InLookup[", strnum, "] dlclose for ", plugin.s, " failed: ", error, 0);
		return (-1);
	}
	return (status);
}
#endif

int
ProcessInFifo(int instNum)
{
	int             i, rfd, wfd, bytes, status, idx, pipe_size, readTimeout, writeTimeout, relative, use_btree,
					max_btree_count, fd, match;
	INENTRY        *in, *re, *retval;
	struct passwd  *pw;
	static stralloc InFifo = {0}, pwbuf = {0}, host_path = {0}, line = {0};
	char            tmp[FMT_ULONG], inbuf[512];
	char           *ptr, *fifoName, *sysconfdir, *infifo_dir, *controldir, *QueryBuf, *email, *myFifo,
				   *remoteip, *infifo, *local_ip, *cntrl_host, *real_domain;
	void            (*pstat) ();
	void           *(*search_func) (const void *key, void *const *rootp, int (*compar)(const void *, const void *));
	time_t          prev_time = 0l;
	substdio        ssin;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif
#ifdef ENABLE_ENTERPRISE
	mdir_t          count;
	long            ucount;
#endif

	_debug = (env_get("DEBUG") ? 1 : 0);
	start_time = time(0);
#ifdef DARWIN
	signal(SIGTERM, (void(*)()) sig_term);
	signal(SIGUSR1, (void(*)()) sig_usr1);
	signal(SIGUSR2, (void(*)()) sig_usr2);
	signal(SIGHUP, (void(*)()) sig_hup);
	signal(SIGINT, (void(*)()) sig_int);
#else
	signal(SIGTERM, (void(*)()) sig_hand);
	signal(SIGUSR1, (void(*)()) sig_hand);
	signal(SIGUSR2, (void(*)()) sig_hand);
	signal(SIGHUP, (void(*)()) sig_hand);
	signal(SIGINT, (void(*)()) sig_hand);
#endif
#ifdef ENABLE_ENTERPRISE
	for (;;) {
		if ((count = count_table("indimail", 0)) == -1) {
			flush_stack();
			out("ProcessInFifo", "InLookup[");
			strnum[fmt_uint(strnum, instNum)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", "] PPID ");
			strnum[fmt_ulong(strnum, getppid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " PID ");
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " unable to get count\n");
			flush("ProcessInFifo");
			sleep(5);
			continue;
		}
		iclose();
		getEnvConfigLong(&ucount, "MAXUSERS", 10000);
		if (count > ucount && (idx = do_startup(instNum))) {
			out("ProcessInFifo", "enterprise version requires plugin\n");
			flush("ProcessInFifo");
			if (idx)
				out("ProcessInFifo", "invalid plugin\n");
			flush("ProcessInFifo");
			sleep(5);
			return (-1);
		}
		out("ProcessInFifo", "InLookup[");
		strnum[fmt_uint(strnum, instNum)] = 0;
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", "] PPID ");
		strnum[fmt_uint(strnum, getppid())] = 0;
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " PID ");
		strnum[fmt_uint(strnum, getpid())] = 0;
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " with ");
		strnum[fmt_uint(strnum, count)] = 0;
		out("ProcessInFifo", strnum);
		out("ProcessInFifo", " users\n");
		flush("ProcessInFifo");
		break;
	}
#else
	out("ProcessInFifo", "InLookup[");
	strnum[fmt_uint(strnum, instNum)] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", "] PPID ");
	strnum[fmt_uint(strnum, getppid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", " PID ");
	strnum[fmt_uint(strnum, getpid())] = 0;
	out("ProcessInFifo", strnum);
	out("ProcessInFifo", "\n");
	flush("ProcessInFifo");
#endif
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
	relative = *infifo_dir == '/' ? 0 : 1;
	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&InFifo, infifo) ||
				!stralloc_0(&InFifo))
			die_nomem();
	} else {
		if (relative) {
			if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
					!stralloc_cats(&InFifo, infifo_dir) ||
					!stralloc_append(&InFifo, "/") ||
					!stralloc_cats(&InFifo, infifo) ||
					!stralloc_0(&InFifo))
				die_nomem();
		} else {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
			if (!stralloc_copys(&InFifo, infifo_dir) ||
					!stralloc_append(&InFifo, "/") ||
					!stralloc_cats(&InFifo, infifo) ||
					!stralloc_0(&InFifo))
				die_nomem();
		}
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getTimeoutValues(&readTimeout, &writeTimeout, sysconfdir, controldir);
	/*- Open the Fifos */
	if (FifoCreate(InFifo.s) == -1) {
		strerr_warn3("InLookup: FifoCreate: ", InFifo.s, ": ", &strerr_sys);
		return (-1);
	} else
	if ((rfd = open(InFifo.s, O_RDWR, 0)) == -1) {
		strerr_warn3("InLookup: open_write: ", InFifo.s, ": ", &strerr_sys);
		return (-1);
	} else 
	if ((pipe_size = fpathconf(rfd, _PC_PIPE_BUF)) == -1) {
		strerr_warn3("InLookup: fpathconf _PC_PIPE_BUF: ", InFifo.s, ": ", &strerr_sys);
		return (-1);
	} else
	if (!(QueryBuf = (char *) alloc(pipe_size * sizeof(char))))
		die_nomem();
	user_query_count = relay_query_count = pwd_query_count = limit_query_count = alias_query_count = dom_query_count = 0;
#ifdef CLUSTERED_SITE
	host_query_count = 0;
#endif
	if (!(local_ip = get_local_ip(PF_INET))) {
		local_ip = "127.0.0.1";
		strerr_warn1("ProcessInFifo: get_local_ip failed. using localhost", 0);
	}
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
		strerr_warn1("InLookup: signal: ", &strerr_sys);
		return (-1);
	}
	ptr = env_get("USE_BTREE");
	use_btree = ((ptr && *ptr == '1') ? 1 : 0);
	scan_uint((ptr = env_get("MAX_BTREE_COUNT")) && *ptr ? ptr : "-1", (unsigned int *) &max_btree_count);
	search_func = (void *) tsearch; /*- this adds a record if not found */
	fifoName = InFifo.s;
	match = str_rchr(fifoName, '/');
	if (fifoName[match])
		fifoName = InFifo.s + match + 1;
	for (bytes = 0;getppid() != 1;) {
		if ((idx = read(rfd, (char *) &bytes, sizeof(int))) == -1) {
			strnum[fmt_uint(strnum, errno)] = 0;
			out("ProcessInFifo", "errno = ");
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", "\n");
			flush("ProcessInFifo");
#ifdef ERESTART
			if (errno != error_intr && errno != error_restart)
#else
			if (errno != error_intr)
#endif
			{
				strerr_warn1("InLookup: read: ", &strerr_sys);
				sleep(1);
			}
			continue;
		} else
		if (!idx) {
			close(rfd);
			if ((rfd = open_write(InFifo.s)) == -1) {
				strerr_warn3("InLookup: open_write: ", InFifo.s, ": ", &strerr_sys);
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		} else
		if (bytes > pipe_size) {
			errno = EMSGSIZE;
			strnum[fmt_uint(strnum, bytes)] = 0;
			tmp[fmt_uint(tmp, pipe_size)] = 0;
			strerr_warn5("InLookup: bytes ", strnum, ", pipe_size ", tmp, ": ", &strerr_sys);
			continue;
		} else
		if ((idx = timeoutread(readTimeout, rfd, QueryBuf, bytes)) == -1) {
			strerr_warn1("InLookup: read-int: ", &strerr_sys);
			continue;
		} else
		if (!idx) {
			close(rfd);
			if ((rfd = open_write(InFifo.s)) == -1) {
				strerr_warn3("InLookup: open_write: ", InFifo.s, ": ", &strerr_sys);
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		}
		if (verbose || _debug)
			prev_time = time(0);
#ifdef CLUSTERED_SITE
		if (relative) {
			if (!stralloc_copys(&host_path, sysconfdir) ||
					!stralloc_append(&host_path, "/") ||
					!stralloc_cats(&host_path, controldir) ||
					!stralloc_catb(&host_path, "/host.cntrl", 11) ||
					!stralloc_0(&host_path))
				die_nomem();
		} else {
			if (!stralloc_copys(&host_path, controldir) ||
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
				signal(SIGPIPE, pstat);
				return (-1);
			}
			if (in_mysql_ping(&mysql[0])) {
				strerr_warn3("mysql_ping: ", (char *) in_mysql_error(&mysql[0]) ,": Reconnecting to central db...", 0);
				in_mysql_close(&mysql[0]);
				isopen_cntrl = 0;
				if (open_central_db(cntrl_host)) {
					strerr_warn1("InLookup: Unable to open central db", 0);
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
			out("ProcessInFifo", fifoName);
			out("ProcessInFifo", "->");
			out("ProcessInFifo", myFifo);
			out("ProcessInFifo", ", Bytes ");
			strnum[fmt_int(strnum, bytes)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", ", Query [");
			out("ProcessInFifo", query_type(*QueryBuf));
			out("ProcessInFifo", "], User ");
			out("ProcessInFifo", email);
			out("ProcessInFifo", ", RemoteIp ");
			out("ProcessInFifo", *QueryBuf == 2 ? remoteip : "N/A");
			out("ProcessInFifo", "\n");
			flush("ProcessInFifo");
		}
		if ((wfd = open_write(myFifo)) == -1) {
			strerr_warn5(errno == error_acces ?  "InLookup: open-write: " : "InLookup: open-probably-timeout: ",
					myFifo, ": QueryType ", query_type(*QueryBuf), ": ", &strerr_sys);
			if (errno != ENOENT && unlink(myFifo))
				strerr_warn3("InLookup: unlink: ", myFifo, ": ", &strerr_sys);
			continue;
		} else
		if (unlink(myFifo)) /*- make this invisible */
			strerr_warn3("InLookup: unlink: ", myFifo, ": ", &strerr_sys);
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
							out("ProcessInFifo", fifoName);
							out("ProcessInFifo", "->");
							out("ProcessInFifo", myFifo);
							out("ProcessInFifo", ", cache hit\n");
							flush("ProcessInFifo");
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
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-UserInLookup: ", &strerr_sys);
				close(wfd);
				break;
			case RELAY_QUERY:
				status = RelayInLookup(email, remoteip);
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-RelayInLookup: ", &strerr_sys);
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
							out("ProcessInFifo", fifoName);
							out("ProcessInFifo", "->");
							out("ProcessInFifo", myFifo);
							out("ProcessInFifo", ", cache hit\n");
							flush("ProcessInFifo");
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
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-findmdahost: ", &strerr_sys);
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1)
					strerr_warn1("InLookup: write-findmdahost: ", &strerr_sys);
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
							out("ProcessInFifo", fifoName);
							out("ProcessInFifo", "->");
							out("ProcessInFifo", myFifo);
							out("ProcessInFifo", ", cache hit\n");
							flush("ProcessInFifo");
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
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-AliasInLookup: ", &strerr_sys);
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1)
					strerr_warn1("InLookup: write-AliasInLookup: ", &strerr_sys);
				close(wfd);
				break;
			case PWD_QUERY:
				i = str_rchr(email, '@');
				if (!email[i]) {
					if (!(real_domain = env_get("DEFAULT_DOMAIN")))
						real_domain = DEFAULT_DOMAIN;
				} else
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
								out("ProcessInFifo", fifoName);
								out("ProcessInFifo", "->");
								out("ProcessInFifo", myFifo);
								out("ProcessInFifo", ", cache hit\n");
								flush("ProcessInFifo");
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
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-PwdInLookup: ", &strerr_sys);
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, pwbuf.s, bytes) == -1)
					strerr_warn1("InLookup: write-PwdInLookup: ", &strerr_sys);
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
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-VlimitInLookup: ", &strerr_sys);
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, (char *) &limits, bytes) == -1)
					strerr_warn1("InLookup: write-VlimitInLookup: ", &strerr_sys);
				close(wfd);
				break;
#endif
			case DOMAIN_QUERY:
				if (!(real_domain = get_real_domain(email)))
					real_domain = email;
				bytes = str_len(real_domain) + 1;
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					strerr_warn1("InLookup: write-get_real_domain: ", &strerr_sys);
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, real_domain, bytes) == -1)
					strerr_warn1("InLookup: write-get_real_domain: ", &strerr_sys);
				close(wfd);
				break;
		} /*- switch(*QueryBuf) */
		if (verbose || _debug) {
			strnum[fmt_ulong(strnum, time(0) - prev_time)] = 0;
			out("ProcessInFifo", strnum);
			out("ProcessInFifo", " ");
			out("ProcessInFifo", query_type(*QueryBuf));
			out("ProcessInFifo", " -> ");
			out("ProcessInFifo", myFifo);
			out("ProcessInFifo", "\n");
			flush("ProcessInFifo");
		}
	} /*- for (QueryBuf = (char *) 0;;) */
	signal(SIGPIPE, pstat);
	return (1);
}
