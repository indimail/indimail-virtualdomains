/*
 * $Log: runcmmd.c,v $
 * Revision 1.4  2022-12-18 19:27:45+05:30  Cprogrammer
 * handle SIGCONT
 *
 * Revision 1.3  2022-05-10 20:01:40+05:30  Cprogrammer
 * use headers from include path
 *
 * Revision 1.2  2021-06-11 17:01:21+05:30  Cprogrammer
 * replaced MakeArgs() with makeargs() from libqmail
 *
 * Revision 1.1  2019-04-18 07:59:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#include <makeargs.h>
#endif
#include "variables.h"

#ifndef lint
static char     sccsid[] = "$Id: runcmmd.c,v 1.4 2022-12-18 19:27:45+05:30 Cprogrammer Exp mbhangui $";
#endif

int
runcmmd(char *cmmd, int useP)
{
	char          **argv;
	int             status, i, retval;
	pid_t           pid;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	void            (*pstat[2]) ();

	switch ((pid = fork()))
	{
	case -1:
		exit(1);
	case 0:
		if (!(argv = makeargs(cmmd)))
			exit(1);
		if (useP)
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		strerr_die2sys(111, *argv, ": ");
	default:
		break;
	}
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	for (retval = -1;;) {
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (i != pid)
			continue;
		if (WIFSTOPPED(status) || WIFCONTINUED(status)) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_uint(strnum2, WIFSTOPPED(status) ? WSTOPSIG(status) : SIGCONT)] = 0;
				strerr_warn3(strnum1, WIFSTOPPED(status) ? " stopped by signal " : " started by signal ", strnum2, 0);
			}
			continue;
		} else
		if (WIFSIGNALED(status)) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_uint(strnum2, WTERMSIG(status))] = 0;
				strerr_warn3(strnum1, ": killed by signal ", strnum2, 0);
			}
			retval = -1;
		} else
		if (WIFEXITED(status)) {
			retval = WEXITSTATUS(status);
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_uint(strnum2, retval < 0 ? 0 - retval : retval)] = 0;
				strerr_warn4(strnum1, ": normal exit return status", retval < 0 ? " -" : " ", strnum2, 0);
			}
		}
		break;
	}
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	return (retval);
}
