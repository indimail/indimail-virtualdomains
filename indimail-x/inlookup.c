/*
 * $Log: inlookup.c,v $
 * Revision 1.4  2020-10-01 18:24:22+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.3  2020-04-01 18:55:49+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-22 23:18:13+05:30  Cprogrammer
 * replace exit() with _exit()
 * replaced atoi with scan_int
 *
 * Revision 1.1  2019-04-20 09:18:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: inlookup.c,v 1.4 2020-10-01 18:24:22+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#include <scan.h>
#include <fmt.h>
#include <sig.h>
#include <env.h>
#include <alloc.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "ProcessInFifo.h"
#include "common.h"

#define FATAL   "inlookup: fatal: "

extern int      btree_count;

struct pidtab
{
	int pid;
	stralloc infifo;
};
struct pidtab  *pid_table;
static int      inst_count = 0;

static void
die_nomem()
{
	strerr_warn1("inlookup: out of memory", 0);
	_exit(111);
}

static int
fork_child(char *infifo, int instNum)
{
	int             pid, i;
	char            strnum[FMT_ULONG];

	switch(pid = fork())
	{
		case -1:
			strerr_warn1("inlookup: fork: ", &strerr_sys);
			return (-1);
		case 0:
#ifdef DARWIN
			signal(SIGTERM, SIG_DFL);
			signal(SIGUSR1, SIG_DFL);
#else
			sig_catch(SIGTERM, SIG_DFL);
			sig_catch(SIGUSR1, SIG_DFL);
#endif
			if (!stralloc_copys(&pid_table[instNum].infifo, infifo) ||
					!stralloc_append(&pid_table[instNum].infifo, ".") ||
					!stralloc_catb(&pid_table[instNum].infifo, strnum, fmt_uint(strnum, instNum + 1)) ||
					!stralloc_0(&pid_table[instNum].infifo))
				die_nomem();
			if (!env_put2("INFIFO", pid_table[instNum].infifo.s))
				strerr_die4sys(111, FATAL, "env_put2: INFIFO=", pid_table[instNum].infifo.s, ": ");
			out("inlookup", "InLookup[");
			strnum[fmt_uint(strnum, instNum + 1)] = 0;
			out("inlookup", strnum);
			out("inlookup", "] PPID ");
			strnum[fmt_ulong(strnum, getppid())] = 0;
			out("inlookup", strnum);
			out("inlookup", " PID ");
			strnum[fmt_ulong(strnum, getpid())] = 0;
			out("inlookup", strnum);
			out("inlookup", " Ready with INFIFO=");
			out("inlookup", infifo);
			out("inlookup", ".");
			strnum[fmt_uint(strnum, instNum + 1)] = 0;
			out("inlookup", strnum);
			out("inlookup", "\n");
			flush("inlookup");
			i = ProcessInFifo(instNum + 1);
			sleep(5);
			_exit(i);
		default:
			pid_table[instNum].pid = pid;
			if (!stralloc_copys(&pid_table[instNum].infifo, infifo) ||
					!stralloc_append(&pid_table[instNum].infifo, ".") ||
					!stralloc_catb(&pid_table[instNum].infifo, strnum, fmt_uint(strnum, instNum + 1)) ||
					!stralloc_0(&pid_table[instNum].infifo))
				die_nomem();
			break;
	}
	return (pid);
}

#ifdef DARWIN
static void
isig_usr1()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGUSR1);
	}
	signal(SIGUSR1, isig_usr1);
	errno = error_intr;
	return;
}

static void
isig_usr2()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGUSR2);
	}
	signal(SIGUSR2, isig_usr2);
	errno = error_intr;
	return;
}

static void
isig_hup()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGHUP);
	}
	signal(SIGHUP, isig_hup);
	errno = error_intr;
	return;
}

static void
isig_int()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGINT);
	}
	signal(SIGINT, isig_int);
	errno = error_intr;
	return;
}

static void
isig_term()
{
	int             idx;

	sig_block(SIGTERM);
	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGTERM);
	}
	_exit(1);
}
#else
static void
sig_hand(sig, code, scp, addr)
	int             sig, code;
	struct sigcontext *scp;
	char           *addr;
{
	int             idx;

	if (sig == SIGTERM)
		sig_block(sig);
	for (idx = 0; idx < inst_count; idx++) {
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, sig);
	}
	if (sig != SIGTERM) {
		sig_catch(sig, (void(*)()) sig_hand);
		errno = error_intr;
		return;
	} else
		_exit(1);
}
#endif /*- #ifdef DARWIN */

int
main(int argc, char **argv)
{
	int             idx, pid, wStat, tmp_stat;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], strnum3[FMT_ULONG];
	char           *infifo, *ptr, *instance = "1", *seconds_active = "0";

	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	ptr = argv[0];
	idx = str_rchr(ptr, '/');
	if (ptr[idx])
		ptr = argv[0] + idx + 1;
	for (idx = 1; idx < argc; idx++) {
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'f':
			infifo = *(argv + idx + 1);
			break;
		case 'i':
			instance = *(argv + idx + 1);
			break;
		case 'c':
			seconds_active = *(argv + idx + 1);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn3("USAGE: ", ptr, " [-f infifo] [-i instance] [-c activeSecs] [-v]", 0);
			return (1);
		}
	}
	scan_uint(instance, (unsigned int *) &inst_count);
	if (inst_count > 1) {
		if (!pid_table) {
			if (!(pid_table = (struct pidtab *) alloc(sizeof(struct pidtab) * inst_count)))
				die_nomem();
			for (idx = 0; idx < inst_count; idx++) {
				pid_table[idx].pid = -1;
				pid_table[idx].infifo.s = 0;
				pid_table[idx].infifo.a = 0;
				pid_table[idx].infifo.len = 0;
			}
		}
#ifdef DARWIN
		signal(SIGTERM, isig_term);
		signal(SIGUSR1, isig_usr1);
		signal(SIGUSR2, isig_usr2);
		signal(SIGHUP, isig_hup);
		signal(SIGINT, isig_int);
#else
		sig_catch(SIGTERM, sig_hand);
		sig_catch(SIGUSR1, sig_hand);
		sig_catch(SIGUSR2, sig_hand);
		sig_catch(SIGHUP, sig_hand);
		sig_catch(SIGINT, sig_hand);
#endif
		scan_int(seconds_active, &tmp_stat);
		if (tmp_stat > 0) {
			if ((wStat = cache_active_pwd(tmp_stat)) == -1)
				return (1);
			else
			if (!wStat) {
				out("inlookup", "cached ");
				strnum1[fmt_uint(strnum1, btree_count)] = 0;
				out("inlookup", strnum1);
				out("inlookup", " records\n");
				flush("inlookup");
			}
		}
		for (idx = 0; idx < inst_count; idx++) {
			if (fork_child(infifo, idx) == -1) /*- parent returns */
				return (-1);
		}
		for (;;) {
			pid = wait(&wStat);
#ifdef ERESTART
			if (pid == -1 && (errno == error_intr || errno == error_restart))
#else
			if (pid == -1 && errno == error_intr)
#endif
				continue;
			else
			if (pid == -1)
				break;
			if (WIFSTOPPED(wStat) || WIFSIGNALED(wStat)) {
				for (idx = 0; idx < inst_count; idx++) {
					if (pid_table[idx].pid == pid) {
						strnum1[fmt_ulong(strnum1, idx + 1)] = 0;
						strnum2[fmt_ulong(strnum1, pid)] = 0;
						strnum3[fmt_int(strnum3, WIFSIGNALED(wStat) ? WTERMSIG(wStat) : (WIFSTOPPED(wStat) ? WSTOPSIG(wStat) : -1))] = 0;
						strerr_warn6("inlookup[", strnum1, "]: child [", strnum2, "] died with signal ", strnum3, 0);
						if (fork_child(infifo, idx) == -1) {
							strnum1[fmt_ulong(strnum1, idx + 1)] = 0;
							strerr_warn3("inlookup[", strnum1, "]: Help!! Unable to start process", 0);
							pid_table[idx].pid = -1;
						}
						break;
					}
				}
				if (idx == inst_count) {
					strnum2[fmt_ulong(strnum1, pid)] = 0;
					strerr_warn3("inlookup: ", strnum2, " crashed. Unable to find slot", 0);
				}
			} else
			if (WIFEXITED(wStat)) {
				tmp_stat = WEXITSTATUS(wStat);
				for (idx = 0; idx < inst_count; idx++) {
					if (pid_table[idx].pid == pid) {
						strnum1[fmt_ulong(strnum1, idx + 1)] = 0;
						strnum2[fmt_ulong(strnum1, pid)] = 0;
						strnum3[fmt_int(strnum3, tmp_stat)] = 0;
						strerr_warn6("inlookup[", strnum1, "]: child [", strnum2, "] died with status ", strnum3, 0);
						if (fork_child(infifo, idx) == -1) {
							strnum1[fmt_ulong(strnum1, idx + 1)] = 0;
							strerr_warn3("inlookup[", strnum1, "]: Help!! Unable to start process", 0);
							pid_table[idx].pid = -1;
						}
						break;
					}
				}
				if (idx == inst_count) {
					strnum2[fmt_ulong(strnum1, pid)] = 0;
					strerr_warn3("inlookup: ", strnum2, " crashed. Unable to find slot", 0);
				}
			}
		}
		for (idx = 0; idx < inst_count; idx++) {
			if (pid_table[idx].pid == -1) {
				if (fork_child(infifo, idx) == -1) {
					strnum1[fmt_ulong(strnum1, idx + 1)] = 0;
					strerr_warn3("inlookup[", strnum1, "]: Help!! Unable to start process", 0);
					pid_table[idx].pid = -1;
				}
			}
		}
		return (-1);
	} else {
		if (!env_put2("INFIFO", infifo))
			strerr_die4sys(111, FATAL, "env_put2: INFIFO=", infifo, ": ");
		out("inlookup", "InLookup INFIFO=");
		out("inlookup", infifo);
		out("inlookup", "\n");
		flush("inlookup");
		return (ProcessInFifo(0));
	}
	return (0);
}
#else
#include <sterrr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (0);
}
#endif
