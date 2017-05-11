/*
 * $Log: svscan.c,v $
 * Revision 1.8  2017-05-11 13:30:34+05:30  Cprogrammer
 * added option to run initialization command via INITCMD env variable
 *
 * Revision 1.7  2017-04-18 08:59:53+05:30  Cprogrammer
 * lock service directory using file .svlock to prevent multiple svscan runs
 *
 * Revision 1.6  2011-08-05 14:31:05+05:30  Cprogrammer
 * create STATUSFILE on startup and delete on shutdown
 *
 * Revision 1.5  2011-05-26 23:17:52+05:30  Cprogrammer
 * open svscan log only if SCANLOG is defined
 *
 * Revision 1.4  2008-06-07 13:54:28+05:30  Cprogrammer
 * added logging of svscan through external loggers like multilog
 *
 * Revision 1.3  2004-10-22 20:31:19+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2003-10-11 09:31:58+05:30  Cprogrammer
 * added scan.h for scan_ulong() prototype
 * changed wait to unsigned long
 *
 * Revision 1.1  2003-10-11 09:28:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "direntry.h"
#include "strerr.h"
#include "error.h"
#include "wait.h"
#include "coe.h"
#include "fd.h"
#include "env.h"
#include "str.h"
#include "byte.h"
#include "scan.h"
#include "fmt.h"
#include "pathexec.h"
#include "auto_sysconfdir.h"

#define SERVICES 1000
#define SVLOCK   ".svlock"

#define WARNING "svscan: warning: "
#define FATAL "svscan: fatal: "
#define INFO  "svscan: info: "

#ifndef SVSCANINFO
#define SVSCANINFO ".svscan"  /* must begin with dot ('.') */
#endif

struct
{
	unsigned long   dev;
	unsigned long   ino;
	int             flagactive;
	int             flaglog;
	int             pid;		/*- 0 if not running */
	int             pidlog;		/*- 0 if not running */
	int             pi[2];		/*- defined if flaglog */
} x[SERVICES];
int             numx = 0;
char            fnlog[260];

void
init_cmd(char *cmmd, int dowait, char *sdir)
{
	int             child, r, wstat;
	int             fdsourcedir = -1;
	char           *args[4];

	/*- save the current dir */
	if ((fdsourcedir = open(".", O_RDONLY|O_NDELAY, 0)) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (chdir(auto_sysconfdir) == -1)
		strerr_die4sys(111, WARNING, "unable to switch to ", auto_sysconfdir, ": ");
	if (access("resolv.conf", R_OK)) {
		if (fchdir(fdsourcedir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsourcedir);
		return;
	}
	if (fchdir(fdsourcedir) == -1)
		strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
	close(fdsourcedir);
	switch (child = fork())
	{
	case -1:
		strerr_warn2(WARNING, "unable to fork for init command: ", &strerr_sys);
		return;
	case 0:
		args[0] = "/bin/sh";
		args[1] = "-c";
		args[2] = cmmd && *cmmd ? cmmd : ".svscan/run";
		args[3] = 0;
		execv(*args, args);
		strerr_die4sys(111, WARNING, "unable to run", cmmd, ": ");
	default:
		break;
	}
	for (;;) {
		r = dowait ? wait_pid(&wstat, child) : wait_nohang(&wstat);
		if (!r || (dowait && r == child))
			break;
		if (r == -1) {
			if (errno == error_intr)
				continue;		/*- impossible */
			break;
		}
	}
	return;
}

void
start(char *fn)
{
	unsigned int    fnlen;
	struct stat     st;
	int             child;
	int             i;
	char           *args[3];

	if (fn[0] == '.' && str_diff(fn, SVSCANINFO))
		return;
	if (stat(fn, &st) == -1)
	{
		strerr_warn4(WARNING, "unable to stat ", fn, ": ", &strerr_sys);
		return;
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR)
		return;
	for (i = 0; i < numx; ++i)
	{
		if (x[i].ino == st.st_ino && x[i].dev == st.st_dev)
			break;
	}
	if (i == numx)
	{
		if (numx >= SERVICES)
		{
			strerr_warn4(WARNING, "unable to start ", fn, ": running too many services", 0);
			return;
		}
		x[i].ino = st.st_ino;
		x[i].dev = st.st_dev;
		/*(fn[0]=='.' here only if SVSCANINFO; if so only supervise log/ subdir)*/
		x[i].pid = (fn[0] != '.') ? 0 : -1;
		x[i].pidlog = 0;
		x[i].flaglog = 0;
		fnlen = str_len(fn);
		if (fnlen <= 255)
		{
			byte_copy(fnlog, fnlen, fn);
			byte_copy(fnlog + fnlen, 5, "/log");
			if (stat(fnlog, &st) == 0)
				x[i].flaglog = 1;
			else
			if (errno != error_noent)
			{
				strerr_warn4(WARNING, "unable to stat ", fn, "/log: ", &strerr_sys);
				return;
			}
		}
		if (x[i].flaglog)
		{
			if (pipe(x[i].pi) == -1)
			{
				strerr_warn4(WARNING, "unable to create pipe for ", fn, ": ", &strerr_sys);
				return;
			}
			coe(x[i].pi[0]);
			coe(x[i].pi[1]);
		}
		++numx;
	}
	x[i].flagactive = 1;
	if (!x[i].pid)
	{
		switch (child = fork())
		{
		case -1:
			strerr_warn4(WARNING, "unable to fork for ", fn, ": ", &strerr_sys);
			return;
		case 0:
			if (x[i].flaglog)
				if (fd_move(1, x[i].pi[1]) == -1)
					strerr_die4sys(111, WARNING, "unable to set up descriptors for ", fn, ": ");
			args[0] = "supervise";
			args[1] = fn;
			args[2] = 0;
			pathexec_run(*args, args, environ);
			strerr_die4sys(111, WARNING, "unable to start supervise ", fn, ": ");
		default:
			x[i].pid = child;
		}
	}
	if (x[i].flaglog && !x[i].pidlog)
	{
		switch (child = fork())
		{
		case -1:
			strerr_warn4(WARNING, "unable to fork for ", fn, "/log: ", &strerr_sys);
			return;
		case 0:
			if (fd_move(0, x[i].pi[0]) == -1)
				strerr_die4sys(111, WARNING, "unable to set up descriptors for ", fn, "/log: ");
			if (chdir(fn) == -1)
				strerr_die4sys(111, WARNING, "unable to switch to ", fn, ": ");
			args[0] = "supervise";
			args[1] = "log";
			args[2] = 0;
			pathexec_run(*args, args, environ);
			strerr_die4sys(111, WARNING, "unable to start supervise ", fn, "/log: ");
		default:
			x[i].pidlog = child;
		}
	}
}

void
direrror(void)
{
	strerr_warn2(WARNING, "unable to read directory: ", &strerr_sys);
}

void
doit(void)
{
	DIR            *dir;
	direntry       *d;
	int             i;
	int             r;
	int             wstat;

	for (;;)
	{
		r = wait_nohang(&wstat);
		if (!r)
			break;
		if (r == -1)
		{
			if (errno == error_intr)
				continue;		/*- impossible */
			break;
		}
		for (i = 0; i < numx; ++i)
		{
			if (x[i].pid == r)
			{
				x[i].pid = 0;
				break;
			}
			if (x[i].pidlog == r)
			{
				x[i].pidlog = 0;
				break;
			}
		}
	}
	for (i = 0; i < numx; ++i)
		x[i].flagactive = 0;
	if(!(dir = opendir(".")))
	{
		direrror();
		return;
	}
	for (;;)
	{
		errno = 0;
		d = readdir(dir);
		if (!d)
			break;
		start(d->d_name);
	}
	if (errno)
	{
		direrror();
		closedir(dir);
		return;
	}
	closedir(dir);
	i = 0;
	while (i < numx)
	{
		if (!x[i].flagactive && !x[i].pid && !x[i].pidlog)
		{
			if (x[i].flaglog)
			{
				close(x[i].pi[0]);
				close(x[i].pi[1]);
				x[i].flaglog = 0;
			}
			x[i] = x[--numx];
			continue;
		}
		++i;
	}
}

static int      scannow = 0;

static void
sighup(int i)
{
	scannow = 1;
	signal(SIGHUP, sighup);
	strerr_warn2(INFO, "got sighup", 0);
}

static void
sigterm(int i)
{
	char           *s;

	if ((s = env_get("STATUSFILE")))
		unlink(s);
	unlink(SVLOCK);
	signal(SIGTERM, sigterm);
	strerr_warn2(INFO, "Stopping svscan", 0);
	_exit(0);
}

static void
open_svscan_log(void)
{
	const int       i = numx;
	struct stat     st;
	static char     fn[] = SVSCANINFO;	/*- avoid compiler warning on const string */

	/*
	 * (semi-paranoid; could be moreso) 
	 */
	if (fstat(STDIN_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_RDONLY);
	if (fstat(STDOUT_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_WRONLY);
	if (fstat(STDERR_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_WRONLY);
	if (stat(fn, &st) == 0)
	{
		start(fn);
		if (i + 1 == numx && x[i].pidlog != 0)
		{
			(void) dup2(x[i].pi[1], STDOUT_FILENO);
			(void) dup2(x[i].pi[1], STDERR_FILENO);
			strerr_warn2(INFO, "Starting svscan", 0);
		}
	}
}

int
get_lock(char *sdir)
{
	int             fd, n;
	pid_t           pid;
	char            strnum[FMT_ULONG], buf[8];
	int             fdsourcedir = -1;

	pid = -1;
	if ((fd = open(SVLOCK, O_CREAT|O_WRONLY|O_EXCL, 0644)) >= 0) {
		pid = getpid();
		if (write(fd, (char *) &pid, sizeof(pid_t)) == -1)
			strerr_die2sys(111, FATAL, "unable to write pid to lock file: ");
		close(fd);
		return (0);
	}
	if (errno != error_exist)
		strerr_die4sys(111, FATAL, "unable to obtain lock in ", sdir, ": ");
	/* .svlock exists */
	if ((fd = open(SVLOCK, O_RDONLY, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to read lock in ", sdir, ": ");
	if (read(fd, (char *) &pid, sizeof(pid)) == -1)
		strerr_die2sys(111, FATAL, "unable to get pid from lock: ");
	close(fd);
	errno = 0;
	if (pid == -1 || (kill(pid, 0) == -1 && errno == error_srch)) { /*- process does not exist */
		if (unlink(SVLOCK) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	/*- let us find out if the process is svscan */
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (errno)
		strerr_die4sys(111, FATAL, "unable to get status of pid ", strnum, ": ");

	/*- save the current dir */
	if ((fdsourcedir = open(".", O_RDONLY|O_NDELAY, 0)) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");

	/*- use the /proc filesystem to figure out command name */
	if (chdir("/proc") == -1) { /*- on systems without /proc filesystem, give up */
		strerr_warn4(FATAL, sdir, " running with pid ", strnum, 0);
		_exit (111);
	}
	if (chdir(strnum) == -1) { /*- process is now dead */
		if (fchdir(fdsourcedir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsourcedir);
		if (unlink(SVLOCK) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	/*- open 'comm' to get the command name */
	if ((fd = open("comm", O_RDONLY, 0)) == -1) { /*- process is now dead */
		if (fchdir(fdsourcedir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsourcedir);
		if (unlink(SVLOCK) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	if ((n = read(fd, buf, 7)) != 7) { /*- non-svcan process is running with this pid */
		close(fd);
		if (fchdir(fdsourcedir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsourcedir);
		if (unlink(SVLOCK) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	close(fd);
	if (buf[0] == 's' && buf[1] == 'v' && buf[2] == 's' && 
		buf[3] == 'c' && buf[4] == 'a' && buf[5] == 'n' && buf[6] == '\n') { /*- indeed pid is svscan process */
		strerr_warn4(FATAL, sdir, " running with pid ", strnum, 0);
		_exit (111);
	}
	/*- some non-svscan process is running with pid */
	if (fchdir(fdsourcedir) == -1)
		strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
	close(fdsourcedir);
	if (unlink(SVLOCK) == -1)
		strerr_die2sys(111, FATAL, "unable to delete lock: ");
	return (1);
}

int
main(int argc, char **argv)
{
	unsigned long   wait;
	int             fd;
	char           *s;

	if (argv[0] && argv[1])
	{
		if (chdir(argv[1]) == -1)
			strerr_die4sys(111, FATAL, "unable to chdir to ", argv[1], ": ");
		while (get_lock(argv[1])) ;
	} else
		while (get_lock(".")) ;
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);
	if((s = env_get("SCANINTERVAL")))
		scan_ulong(s, &wait);
	else
		wait = 5;
	if (env_get("SCANLOG"))
		open_svscan_log();
	if ((s = env_get("STATUSFILE"))) {
		if ((fd = open(s, O_CREAT, 0644)) == -1)
			strerr_die4sys(111, FATAL, "unable to open ", s, ": ");
		close(fd);
	}
	if ((s = env_get("INITCMD")))
		init_cmd(s, env_get("WAIT_INITCMD") ? 1 : 0, ".");
	for (;;)
	{
		doit();
		if (wait)
			sleep(wait);
		else
			while (!scannow) sleep(60);
		scannow=0;
	}
}

void
getversion_svscan_c()
{
	static char    *x = "$Id: svscan.c,v 1.8 2017-05-11 13:30:34+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
