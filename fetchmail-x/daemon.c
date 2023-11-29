/*
 * daemon.c -- turn a process into a daemon under POSIX, SYSV, BSD.
 *
 * For license terms, see the file COPYING in this directory.
 */

#include "config.h"
#include "fetchmail.h"

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>	/* get umask(2) prototyped */

#include <unistd.h>

#include <stdlib.h>

#include <termios.h>		/* for TIOCNOTTY under Linux */

/* BSD portability hack */
#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD	SIGCLD
#endif

#include "tunable.h"

static void
sigchld_handler (int sig)
/* process SIGCHLD to obtain the exit code of the terminating process */
{
    int status;

    while (waitpid(-1, &status, WNOHANG) > 0)
	continue; /* swallow 'em up. */
    lastsig = SIGCHLD;
    (void)sig;
}

void null_signal_handler(int sig) { (void)sig; }

SIGHANDLERTYPE set_signal_handler(int sig, SIGHANDLERTYPE handler)
/* 
 * This function is called by other parts of the program to
 * setup the signal handler after a change to the signal context.
 * This is done to improve robustness of the signal handling code.
 * It has the same prototype as signal(2).
 */
{
  SIGHANDLERTYPE rethandler;
  struct sigaction sa_new, sa_old;

  memset (&sa_new, 0, sizeof sa_new);
  sigemptyset (&sa_new.sa_mask);
  sa_new.sa_handler = handler;
  sa_new.sa_flags = 0;
  /* system call should restart on all signals except SIGALRM */
  if (sig != SIGALRM)
      sa_new.sa_flags |= SA_RESTART;
  if (sig == SIGCHLD)
      sa_new.sa_flags |= SA_NOCLDSTOP;
  sigaction(sig, &sa_new, &sa_old);
  rethandler = sa_old.sa_handler;
#if defined(SIGPWR)
  if (sig == SIGCHLD)
     sigaction(SIGPWR, &sa_new, NULL);
#endif
  return rethandler;
}

void deal_with_sigchld(void)
{
  set_signal_handler(SIGCHLD, sigchld_handler);
}

int
daemonize (const char *logfile)
/* detach from control TTY, become process group leader, catch SIGCHLD */
{
  int fd, logfd;
  pid_t childpid;

  /* if we are started by init (process 1) via /etc/inittab we needn't 
     bother to detach from our process group context */

  if (getppid() == 1) 
    goto nottyDetach;

  /* Ignore BSD terminal stop signals */
#ifdef 	SIGTTOU
  set_signal_handler(SIGTTOU, SIG_IGN);
#endif
#ifdef	SIGTTIN
  set_signal_handler(SIGTTIN, SIG_IGN);
#endif
#ifdef	SIGTSTP
  set_signal_handler(SIGTSTP, SIG_IGN);
#endif

  /* In case we were not started in the background, fork and let
     the parent exit.  Guarantees that the child is not a process
     group leader */

  if ((childpid = fork()) < 0) {
    report(stderr, "fork (%s)\n", strerror(errno));
    return(PS_IOERR);
  }
  else if (childpid > 0) 
    exit(0);  /* parent */

  
  /* Make ourselves the leader of a new process group with no
     controlling terminal */

  /* POSIX makes this soooo easy to do */
  if (setsid() < 0) {
    report(stderr, "setsid (%s)\n", strerror(errno));
    return(PS_IOERR);
  }

nottyDetach:

  (void)close(0);

  /* Reopen stdin descriptor on /dev/null */
  if (open("/dev/null", O_RDWR) < 0) {   /* stdin */
    report(stderr, "cannot open /dev/null: %s\n", strerror(errno));
    return(PS_IOERR);
  }

  if (logfile)
  {
      if ((logfd = open(logfile, O_CREAT|O_WRONLY|O_APPEND, 0666)) < 0) {	/* stdout */
	  report(stderr, "cannot open %s: %s\n", logfile, strerror(errno));
	  return PS_IOERR;
      }
  } else
      logfd = 0;    /* else use /dev/null */

  /* Close any/all open file descriptors */
  fd = sysconf(_SC_OPEN_MAX);
  while (fd >= 1) {
      if (fd != logfd)
	  close(fd);	/* not checking this should be safe, no writes */
      -- fd;
  }

  if (dup(logfd) < 0						/* stdout */
	  || ((logfd == 0 || logfd >= 3) && dup(logfd) < 0)) {	/* stderr */
      report(stderr, "dup(): %s\n", strerror(errno));
      return(PS_IOERR);
  }

  /* move to root directory, so we don't prevent filesystem unmounts */
  if (chdir("/")) {
	  report(stderr, "chdir(\"/\"): %s\n", strerror(errno));
	  return PS_IOERR;
  }

  /* set our umask to something reasonable (we hope) */
#if defined(DEF_UMASK)
  umask(DEF_UMASK);
#else
  umask(022);
#endif

  deal_with_sigchld();

  return(0);
}

flag is_a_file(int fd)
/* is the given fd attached to a file? (used to control logging) */
{
    struct stat stbuf;

    /*
     * We'd like just to return 1 on (S_IFREG | S_IFBLK),
     * but weirdly enough, Linux ptys seem to have S_IFBLK
     * so this test would fail when run on an xterm.
     */
    if (isatty(fd) || fstat(fd, &stbuf))
	return(0);
    else if (stbuf.st_mode & (S_IFREG))
	return(1);
    return(0);
}

/* daemon.c ends here */
