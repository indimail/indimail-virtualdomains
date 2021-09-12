/*
 * $Log: inquerytest.c,v $
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

#ifndef	lint
static char     sccsid[] = "$Id: inquerytest.c,v 1.8 2021-09-12 11:53:22+05:30 Cprogrammer Exp mbhangui $";
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

void
print_limits(struct vlimits *limits)
{
	char            strnum[FMT_ULONG];

	out("inquerytest", "Domain Expiry Date   : ");
	out("inquerytest", limits->domain_expiry == -1 ? "Never Expires\n" : ctime(&limits->domain_expiry));
	out("inquerytest", "Password Expiry Date : ");
	out("inquerytest", limits->passwd_expiry == -1 ? "Never Expires\n" : ctime(&limits->passwd_expiry));
	out("inquerytest", "Max Domain Quota     : ");
	strnum[fmt_long(strnum, limits->diskquota)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Domain Messages  : ");
	strnum[fmt_long(strnum, limits->maxmsgcount)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Default User Quota   : ");
	strnum[fmt_long(strnum, limits->defaultquota)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Default User Messages: ");
	strnum[fmt_long(strnum, limits->defaultmaxmsgcount)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Pop Accounts     : ");
	strnum[fmt_int(strnum, limits->maxpopaccounts)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Aliases          : ");
	strnum[fmt_int(strnum, limits->maxaliases)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Forwards         : ");
	strnum[fmt_int(strnum, limits->maxforwards)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Autoresponders   : ");
	strnum[fmt_int(strnum, limits->maxautoresponders)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "Max Mailinglists     : ");
	strnum[fmt_int(strnum, limits->maxmailinglists)] = 0;
	out("inquerytest", strnum);
	out("inquerytest", "\n");
	out("inquerytest", "GID Flags:\n");
	if (limits->disable_imap != 0)
		out("inquerytest", "  NO_IMAP\n");
	if (limits->disable_smtp != 0)
		out("inquerytest", "  NO_SMTP\n");
	if (limits->disable_pop != 0)
		out("inquerytest", "  NO_POP\n");
	if (limits->disable_webmail != 0)
		out("inquerytest", "  NO_WEBMAIL\n");
	if (limits->disable_passwordchanging != 0)
		out("inquerytest", "  NO_PASSWD_CHNG\n");
	if (limits->disable_relay != 0)
		out("inquerytest", "  NO_RELAY\n");
	if (limits->disable_dialup != 0)
		out("inquerytest", "  NO_DIALUP\n");
	out("inquerytest", "Flags for non postmaster accounts:\n");
	out("inquerytest", "  pop account           : ");
	out("inquerytest", (limits->perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  alias                 : ");
	out("inquerytest", (limits->perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  forward               : ");
	out("inquerytest", (limits->perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  autoresponder         : ");
	out("inquerytest", (limits->perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  mailinglist           : ");
	out("inquerytest", (limits->perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  mailinglist users     : ");
	out("inquerytest", (limits->perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  mailinglist moderators: ");
	out("inquerytest", (limits->perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  domain quota          : ");
	out("inquerytest", (limits->perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", (limits->perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	out("inquerytest", "\n");
	out("inquerytest", "  default quota         : ");
	out("inquerytest", (limits->perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	out("inquerytest", (limits->perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	out("inquerytest", "\n");
	flush("inquerytest");
	return;
}

#ifdef HAVE_QMAIL
no_return
#endif
void
SigChild()
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
			getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
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
		if (access(InFifo.s, F_OK) || (fd = open(InFifo.s, O_WRONLY|O_NONBLOCK)) == -1) {
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
			getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
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
			strnum[fmt_int(strnum, (int) *((int *) dbptr))] = 0;
			out("inquerytest", " has unknown status [");
			out("inquerytest", strnum);
			out("inquerytest", "]\n");
			break;
		}
		flush("inquerytest");
		break;
	case RELAY_QUERY:
		out("inquerytest", email);
		out("inquerytest", " is ");
		out("inquerytest", (int) *((int *) dbptr) == 1 ? "authenticated" : "not authenticated");
		out("inquerytest", " on ");
		out("inquerytest", ipaddr);
		out("inquerytest", "\n");
		flush("inquerytest");
		break;
	case PWD_QUERY:
		pw = (struct passwd *) dbptr;
		out("inquerytest", "pw_name  : ");
		out("inquerytest", pw->pw_name);
		out("inquerytest", "\n");
		out("inquerytest", "pw_passwd: ");
		out("inquerytest", pw->pw_passwd);
		out("inquerytest", "\n");
		out("inquerytest", "pw_uid   : ");
		strnum[fmt_uint(strnum, (unsigned int) pw->pw_uid)] = 0;
		out("inquerytest", strnum);
		out("inquerytest", "\n");
		out("inquerytest", "pw_gid   : ");
		strnum[fmt_uint(strnum, (unsigned int) pw->pw_gid)] = 0;
		out("inquerytest", strnum);
		out("inquerytest", "\n");
		out("inquerytest", "pw_gecos : ");
		out("inquerytest", pw->pw_gecos);
		out("inquerytest", "\n");
		out("inquerytest", "pw_dir   : ");
		out("inquerytest", pw->pw_dir);
		out("inquerytest", "\n");
		out("inquerytest", "pw_shell : ");
		out("inquerytest", pw->pw_shell);
		out("inquerytest", "\n");
		out("inquerytest", "Table    : ");
		getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
		getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
		out("inquerytest", is_inactive ? inactive_table : default_table);
		out("inquerytest", "\n");
		flush("inquerytest");
		break;
	case LIMIT_QUERY:
		lmt = (struct vlimits *) dbptr;
		print_limits(lmt);
		break;
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
		out("inquerytest", email);
		out("inquerytest", ": SMTPROUTE is ");
		out("inquerytest", (char *) dbptr);
		out("inquerytest", "\n");
		flush("inquerytest");
		break;
#endif
	case ALIAS_QUERY:
		if (!*((char *) dbptr)) {
			strerr_warn2(email, ": No aliases\n", 0);
			break;
		}
		out("inquerytest", "Alias List for ");
		out("inquerytest", email);
		out("inquerytest", "\n");
		for (ptr = s = (char *) dbptr; *s; s++) {
			if (*s == '\n') {
				*s = 0;
				out("inquerytest", ptr);
				out("inquerytest", "\n");
				ptr = s + 1;
				*s = '\n';
			}
		}
		out("inquerytest", ptr);
		out("inquerytest", "\n");
		flush("inquerytest");
		break;
	case DOMAIN_QUERY:
		out("inquerytest", email);
		out("inquerytest", ": Real Domain is ");
		out("inquerytest", (char *) dbptr);
		out("inquerytest", "\n");
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
