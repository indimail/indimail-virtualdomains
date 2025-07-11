/*
 * $Id: inquerytest.c,v 1.13 2025-05-13 20:00:38+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <error.h>
#include <env.h>
#include <getEnvConfig.h>
#include <noreturn.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#endif
#include "indimail.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "r_mkdir.h"
#include "inquery.h"
#include "common.h"
#include "FifoCreate.h"
#include "ProcessInFifo.h"
#include "vlimits.h"
#include "print_limits.h"

#ifndef	lint
static char     sccsid[] = "$Id: inquerytest.c,v 1.13 2025-05-13 20:00:38+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "inquerytest: fatal: "
#define WARN    "inquerytest: warning: "

static char    *usage =
	"usage: inquerytest [-v] -q query_type -i infifo email [ipaddr]\n"
	"1   - User         Status Query\n"
	"2   - Relay        Status Query\n"
	"3   - Passwd       Query\n"
	"4   - SmtpRoute    Query\n"
	"5   - Valias       Query\n"
	"6   - Domain Limit Query\n"
	"7   - Domain       Query"
	;

#ifdef HAVE_QMAIL
no_return
#endif
static void
die_nomem()
{
	strerr_warn1("inquerytest: out of memory", 0);
	_exit(111);
}

#ifdef HAVE_QMAIL
no_return
#endif
void
SigChild(int x)
{
	int             status;

	strerr_warn1("inquerytest: InLookup died", 0);
	wait(&status);
	_exit(1);
}

int
main(int argc, char **argv)
{
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits *lmt;
#endif
	void           *dbptr;
	char           *ptr, *s, *infifo = 0, *infifo_dir, *email = 0, *ipaddr = 0;
	static stralloc InFifo = {0};
	char            strnum[FMT_ULONG];
	int             c, query_type = -1, fd = -1, status, fdt = -1;
	pid_t           pid;

	while ((c = getopt(argc, argv, "vq:i:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'q':
			query_type = *optarg - '0';
			break;
		case 'i':
			infifo = optarg;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (query_type == -1)
		strerr_die1x(100, usage);
	if (optind < argc)
		email = argv[optind++];
	if (!email) {
		strerr_warn1("inquerytest: email not specified", 0);
		strerr_die1x(100, usage);
	}
	if (optind < argc)
		ipaddr = argv[optind++];
	switch (query_type)
	{
	case USER_QUERY:
	case RELAY_QUERY:
		if (query_type == RELAY_QUERY && !ipaddr) {
			strerr_warn1("inquerytest: ipaddr must be specified for RELAY query", 0);
			strerr_die1x(100, usage);
		}
	case PWD_QUERY:
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
#endif
	case ALIAS_QUERY:
	case LIMIT_QUERY:
	case DOMAIN_QUERY:
		break;
	default:
		strnum[fmt_uint(strnum, (unsigned int) query_type)] = 0;
		strerr_warn3("inquerytest: Invalid query type [", strnum, "]", 0);
		strerr_die1x(100, usage);
	}
	if (infifo && *infifo) {
		if (*infifo == '/' || *infifo == '.') {
			if (!stralloc_copys(&InFifo, infifo) || !stralloc_0(&InFifo))
				die_nomem();
			InFifo.len--;
		} else {
			getEnvConfigStr(&infifo_dir, "INFIFODIR", INFIFODIR);
			if (*infifo_dir == '/') {
				if (indimailuid == -1 || indimailgid == -1)
					get_indimailuidgid(&indimailuid, &indimailgid);
				if (r_mkdir(infifo_dir, 0775, indimailuid, indimailgid))
					strerr_die4sys(111, FATAL, "r_mkdir: ", infifo_dir, ": ");
				if (!stralloc_copys(&InFifo, infifo_dir) ||
						!stralloc_append(&InFifo, "/") ||
						!stralloc_cats(&InFifo, infifo) ||
						!stralloc_0(&InFifo))
					die_nomem();
			} else {
				if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
						!stralloc_cats(&InFifo, infifo_dir) ||
						!stralloc_append(&InFifo, "/") ||
						!stralloc_cats(&InFifo, infifo) ||
						!stralloc_0(&InFifo))
					die_nomem();
			}
			InFifo.len--;
		}
		if (access(InFifo.s, F_OK) && errno == ENOENT) {
			strerr_warn2("Creating FIFO ", InFifo.s, 0);
			if (FifoCreate(InFifo.s) == -1) {
				strerr_warn3("inquerytest: ", InFifo.s, ": ", &strerr_sys);
				return (1);
			}
			if (!env_put2("INFIFO", InFifo.s))
				strerr_die4sys(111, FATAL, "env_put2: INFIFO=", InFifo.s, ": ");
			switch (pid = fork())
			{
			case -1:
				strerr_warn1("inquerytest: fork: ", &strerr_sys);
				return (1);
			case 0:
				return (ProcessInFifo(0));
			default:
				signal(SIGCHLD, SigChild);
				break;
			}
			if ((fdt = open(InFifo.s, O_RDONLY)) == -1)
				strerr_die4sys(111, FATAL, "open: ", InFifo.s, ": ");
		} else { /*- Fifo is present and open by inlookup */
			if (!env_put2("INFIFO", InFifo.s))
				strerr_die4sys(111, FATAL, "env_put2: INFIFO=", InFifo.s, ": ");
			pid = -1;
			close(fd);
		}
	} else {
		getEnvConfigStr(&infifo, "INFIFO", INFIFO);
		if (*infifo == '/' || *infifo == '.') {
			if (!stralloc_copys(&InFifo, infifo) || !stralloc_0(&InFifo))
				die_nomem();
			InFifo.len--;
		} else {
			getEnvConfigStr(&infifo_dir, "INFIFODIR", INFIFODIR);
			if (*infifo_dir == '/') {
				if (!stralloc_copys(&InFifo, infifo_dir) ||
						!stralloc_append(&InFifo, "/") ||
						!stralloc_cats(&InFifo, infifo) ||
						!stralloc_0(&InFifo))
					die_nomem();
			} else {
				if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
						!stralloc_cats(&InFifo, infifo_dir) ||
						!stralloc_append(&InFifo, "/") ||
						!stralloc_cats(&InFifo, infifo) ||
						!stralloc_0(&InFifo))
					die_nomem();
			}
			InFifo.len--;
		}
		pid = -1;
	}
	if (fdt != -1)
		close(fdt);
	if (!(dbptr = inquery(query_type, email, ipaddr))) {
		if (userNotFound)
			strerr_warn2(email, ": No such user", 0);
		else {
			if (errno)
				strerr_warn2(email, ": No such user: ", &strerr_sys);
			else
				strerr_warn2(email, ": inquery failure", 0);
		}
		signal(SIGCHLD, SIG_IGN);
		if (pid != -1) {
			kill(pid, SIGTERM);
			unlink(InFifo.s);
		}
		if (fd != -1)
			close(fd);
		return (1);
	}
	if (fd != -1)
		close(fd);
	if (pid != -1)
		unlink(InFifo.s);
	switch (query_type)
	{
	case USER_QUERY:
		out("inquerytest", email);
		switch ((int) *((int *) dbptr))
		{
		case 0:
			out("inquerytest", " has all services enabled\n");
			break;
		case 1:
			out("inquerytest", " is Absent on this domain (#5.1.1)\n");
			break;
		case 2:
			out("inquerytest", " is Inactive on this domain (#5.2.1)\n");
			break;
		case 3:
			out("inquerytest", " is Overquota on this domain (#5.2.2)\n");
			break;
		case 4:
			out("inquerytest", " is an alias\n");
			break;
		default:
			subprintfe(subfdout, "inquerytest", " has unknown status [%d]\n", (int) *((int *) dbptr));
			break;
		}
		flush("inquerytest");
		break;
	case RELAY_QUERY:
		subprintfe(subfdout, "inquerytest", "%s is %s on %s\n", email,
				(int) *((int *) dbptr) == 1 ? "authenticated" : "not authenticated",
				ipaddr);
		flush("inquerytest");
		break;
	case PWD_QUERY:
		pw = (struct passwd *) dbptr;
		getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
		getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
		subprintfe(subfdout, "inquerytest", "pw_name  : %s\n", pw->pw_name);
		subprintfe(subfdout, "inquerytest", "pw_passwd: %s\n", pw->pw_passwd);
		subprintfe(subfdout, "inquerytest", "pw_uid   : %u\n", pw->pw_uid);
		subprintfe(subfdout, "inquerytest", "pw_gid   : %u\n", pw->pw_gid);
		subprintfe(subfdout, "inquerytest", "pw_gecos : %s\n", pw->pw_gecos);
		subprintfe(subfdout, "inquerytest", "pw_dir   : %s\n", pw->pw_dir);
		subprintfe(subfdout, "inquerytest", "pw_shell : %s\n", pw->pw_shell);
		subprintfe(subfdout, "inquerytest", "Table    : %s\n", is_inactive ? inactive_table : default_table);
		flush("inquerytest");
		break;
	case LIMIT_QUERY:
		lmt = (struct vlimits *) dbptr;
		print_limits(lmt);
		break;
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
		subprintfe(subfdout, "inquerytest", "%s: SMTPROUTE is %s\n", email, (char *) dbptr);
		flush("inquerytest");
		break;
#endif
	case ALIAS_QUERY:
		if (!*((char *) dbptr)) {
			strerr_warn2(email, ": No aliases\n", 0);
			break;
		}
		subprintfe(subfdout, "inquerytest", "Alias List for %s\n", email);
		for (ptr = s = (char *) dbptr; *s; s++) {
			if (*s == '\n') {
				*s = 0;
				subprintfe(subfdout, "inquerytest", "%s\n", ptr);
				ptr = s + 1;
				*s = '\n';
			}
		}
		subprintfe(subfdout, "inquerytest", "%s\n", ptr);
		flush("inquerytest");
		break;
	case DOMAIN_QUERY:
		subprintfe(subfdout, "inquerytest", "%s: Real Domain is %s\n", email, (char *) dbptr);
		flush("inquerytest");
		break;
	default:
		strerr_warn1("usage: inquerytest query_type email [ipaddr]", 0);
		signal(SIGCHLD, SIG_IGN);
		if (pid != -1)
			kill(pid, SIGTERM);
		return (1);
	}
	signal(SIGCHLD, SIG_IGN);
	if (pid != -1)
		kill(pid, SIGTERM);
	wait(&status);
	return (0);
}

/*
 * $Log: inquerytest.c,v $
 * Revision 1.13  2025-05-13 20:00:38+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.12  2024-05-28 19:25:06+05:30  Cprogrammer
 * use print_limit function from print_limit.c
 *
 * Revision 1.11  2024-05-27 22:51:37+05:30  Cprogrammer
 * change data type to long
 *
 * Revision 1.10  2023-06-08 17:48:31+05:30  Cprogrammer
 * renamed fifo directory from FIFODIR to INFIFODIR
 *
 * Revision 1.9  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.8  2021-09-12 11:53:22+05:30  Cprogrammer
 * removed unused variable controldir
 *
 * Revision 1.7  2021-09-01 18:29:48+05:30  Cprogrammer
 * mark functions not returning as __attribute__ ((noreturn))
 *
 * Revision 1.6  2021-06-09 18:59:10+05:30  Cprogrammer
 * test fifo for read to ensure inlookup process has opened fifo in write mode
 *
 * Revision 1.5  2021-06-09 17:03:49+05:30  Cprogrammer
 * BUG: Fixed SIGSEGV
 *
 * Revision 1.4  2021-02-07 20:30:25+05:30  Cprogrammer
 * minor code optimization
 *
 * Revision 1.3  2020-04-01 18:55:55+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 16:05:57+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:19:53+05:30  Cprogrammer
 * Initial revision
 *
 */
