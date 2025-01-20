/*
 * $Log: pipe_exec.c,v $
 * Revision 1.3  2020-09-28 13:28:20+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.2  2020-09-28 12:49:06+05:30  Cprogrammer
 * display authmodule being executed in debug statements
 *
 * Revision 1.1  2019-04-18 08:16:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <env.h>
#include <fmt.h>
#endif

#ifndef lint
static char     sccsid[] = "$Id: pipe_exec.c,v 1.3 2020-09-28 13:28:20+05:30 Cprogrammer Exp mbhangui $";
#endif

int
pipe_exec(char **argv, char *tmpbuf, int len)
{
	int             pipe_fd[2];
	void            (*pstat) (int);
	char            strnum[FMT_ULONG];

	if (env_get("DEBUG")) {
		strnum[fmt_ulong(strnum, getpid())] = 0;
		strerr_warn6(argv[0], ": pid [", strnum, "], executing authmodule [", argv[1], "]", 0);
	}
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
		strerr_warn1("pipe_exec: signal: ", &strerr_sys);
		return (-1);
	}
	if (pipe(pipe_fd) == -1) {
		strerr_warn1("pipe_exec: pipe: ", &strerr_sys);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	if (dup2(pipe_fd[0], 3) == -1 || dup2(pipe_fd[1], 4) == -1) {
		strerr_warn1("pipe_exec: dup2: ", &strerr_sys);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	if (pipe_fd[0] != 3 && pipe_fd[0] != 4)
		close(pipe_fd[0]);
	if (pipe_fd[1] != 3 && pipe_fd[1] != 4)
		close(pipe_fd[1]);
	if (write(4, tmpbuf, len) != len) {
		strerr_warn1("pipe_exec: write: ", &strerr_sys);
		signal(SIGPIPE, pstat);
		return (-1);
	}
	close(4);
	signal(SIGPIPE, pstat);
	execvp(argv[1], argv + 1);
	strerr_warn3("pipe_exec: ", argv[1], ": ", &strerr_sys);
	return (-1);
}
