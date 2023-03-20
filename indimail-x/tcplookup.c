/*
 * $Log: tcplookup.c,v $
 * Revision 1.5  2023-03-20 10:18:43+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2021-09-12 11:53:07+05:30  Cprogrammer
 * removed redundant multiple initialization of InFifo.len
 *
 * Revision 1.3  2021-05-03 12:47:51+05:30  Cprogrammer
 * initialize rfd, wfd
 *
 * Revision 1.2  2021-02-14 21:40:13+05:30  Cprogrammer
 * include <stdlib.h> for srand()
 *
 * Revision 1.1  2021-02-07 20:40:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <getEnvConfig.h>
#include <env.h>
#include <fmt.h>
#include <scan.h>
#include <stralloc.h>
#include <strerr.h>
#include <substdio.h>
#include <open.h>
#include <getln.h>
#include <error.h>
#include <alloc.h>
#include <str.h>
#endif
#include "FifoCreate.h"
#include "common.h"
#include "variables.h"

#ifndef lint
static char     sccsid[] = "$Id: tcplookup.c,v 1.5 2023-03-20 10:18:43+05:30 Cprogrammer Exp mbhangui $";
#endif

char            strnum[FMT_ULONG];
char           *remote_ip;

static void
die_nomem()
{
	strerr_die1x(111, "tcplookup: out of memory");
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
				if (!line.len)
					*readTimeout = 4;
				line.s[line.len] = 0; /*- remove newline */
			}
			if (line.len)
				scan_uint(line.s, (unsigned int *) readTimeout);
			else
				*readTimeout = 4;
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
				if (!line.len)
					*writeTimeout = 4;
				line.s[line.len] = 0; /*- null terminate */
			}
			if (line.len)
				scan_uint(line.s, (unsigned int *) writeTimeout);
			else
				*writeTimeout = 4;
		}
		close(fd);
	}
	return;
}

static void
cleanup(int rfd, int wfd, void (*sig_pipe_save)(), char *fifo)
{
	int             tmperrno;

	tmperrno = errno;
	if (rfd != -1)
		close(rfd);
	if (wfd != -1)
		close(wfd);
	if (sig_pipe_save)
		signal(SIGPIPE, sig_pipe_save);
	if (fifo)
		unlink(fifo);
	errno = tmperrno;
	return;
}

char           *
prepare_wfifo(int *wfd, int *fifo_len, int *pipe_size, int bytes)
{
	static stralloc InFifo = { 0 };
	char           *infifo_dir, *infifo;
	int             idx, len;

	if (!(infifo = env_get("INFIFO")))
		infifo = INFIFO;
	/*- Open the Fifos */
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&InFifo, infifo) || !stralloc_0(&InFifo))
			die_nomem();
	} else {
		getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
		if (*infifo_dir == '/') {
			if (!stralloc_copys(&InFifo, infifo_dir) ||
					!stralloc_catb(&InFifo, "/", 1) ||
					!stralloc_cats(&InFifo, infifo))
				die_nomem();
		} else {
			if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
					!stralloc_cats(&InFifo, infifo_dir) ||
					!stralloc_catb(&InFifo, "/", 1) ||
					!stralloc_cats(&InFifo, infifo))
				die_nomem();
		}
		for (idx = 1, len = InFifo.len;;idx++) {
			strnum[fmt_ulong(strnum, (unsigned long) idx)] = 0;
			if (!stralloc_catb(&InFifo, ".", 1) ||
					!stralloc_cats(&InFifo, strnum) ||
					!stralloc_0(&InFifo))
				die_nomem();
			if (access(InFifo.s, F_OK))
				break;
			InFifo.len = len;
		}
#ifdef RANDOM_BALANCING
		srand(getpid() + time(0));
		strnum[fmt_ulong(strnum, 1 + (int) ((float) (idx - 1) * rand()/(RAND_MAX + 1.0)))] = 0;
#else
		strnum[fmt_ulong(strnum, 1 + (time(0) % (idx - 1)))] = 0;
#endif
	}
	if (!stralloc_catb(&InFifo, ".", 1) ||
			!stralloc_cats(&InFifo, strnum) ||
			!stralloc_0(&InFifo))
		die_nomem();
	if ((*wfd = open(InFifo.s, O_WRONLY | O_NDELAY, 0)) == -1)
		strerr_die3sys(111, "tcplookup: open: ", InFifo.s, ": ");
	else
	if (bytes > (*pipe_size = fpathconf(*wfd, _PC_PIPE_BUF))) {
		errno = EMSGSIZE;
		strerr_die3sys(111, "tcplookup: fpathconf: ", InFifo.s, ": ");
	}
	*fifo_len = InFifo.len - 1;
	return (InFifo.s);
}

#ifdef MAIN
int
main(int argc, char **argv, char **envp)
#else
int
tcplookup(int argc, char **argv, char **envp)
#endif
{
	int             idx, rfd, wfd, bytes, readTimeout, writeTimeout, fifo_len, pipe_size = 1024;
	char           *ptr, *sysconfdir, *controldir, *rquerybuf, *email, *wfifo, *remoteip;
	char            tmp[FMT_ULONG], inbuf[512];
	static stralloc querybuf = { 0 };
	static stralloc myfifo = { 0 };
	void            (*sig_pipe_save) () = NULL;

#ifndef MAIN
	environ = envp;
#endif
	ptr = argv[0];
	idx = str_rchr(ptr, '/');
	if (ptr[idx])
		ptr = argv[0] + idx + 1;
	for (idx = 1; idx < argc; idx++) {
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn3("USAGE: ", ptr, " [-v]", 0);
			_exit (100);
		}
	}
	remote_ip = env_get("TCPREMOTEIP");
	errout("tcplookup", "tcplookup[");
	errout("tcplookup", remote_ip);
	errout("tcplookup", "] PPID ");
	strnum[fmt_uint(strnum, getppid())] = 0;
	errout("tcplookup", strnum);
	errout("tcplookup", " PID ");
	strnum[fmt_uint(strnum, getpid())] = 0;
	errout("tcplookup", strnum);
	errout("tcplookup", "\n");
	errflush("tcplookup");
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getTimeoutValues(&readTimeout, &writeTimeout, sysconfdir, controldir);
	if (!(rquerybuf = (char *) alloc(pipe_size * sizeof(char))))
		die_nomem();
	for (bytes = 0; getppid() != 1;) {
		if ((idx = read(0, (char *) &bytes, sizeof(int))) == -1) {
#ifdef ERESTART
			if (errno != error_intr && errno != error_restart)
#else
			if (errno != error_intr)
#endif
			{
				strerr_warn1("tcplookup: read: ", &strerr_sys);
				_exit(100);
			}
			continue;
		} else
		if (!idx)
			return (0);
		else
		if (bytes > pipe_size) {
			errno = EMSGSIZE;
			strnum[fmt_uint(strnum, bytes)] = 0;
			tmp[fmt_uint(tmp, pipe_size)] = 0;
			strerr_die5sys(111, "tcplookup: bytes ", strnum, ", pipe_size ", tmp, ": ");
		} else
		if ((idx = timeoutread(readTimeout, 0, rquerybuf, bytes)) == -1) {
			strerr_die1sys(111, "tcplookup: read-int: ");
		} else
		if (!idx)
			strerr_die1x(111, "tcplookup: partial read: "); /*- partial read */
		else
			break;
	} /*- for (bytes = 0; getppid() != 1;) */
	email = rquerybuf + 2;
	for (ptr = email; *ptr; ptr++);
	ptr++;
	/*- myFifo = ptr; */
	for (; *ptr; ptr++);
	ptr++;
	remoteip = ptr;
	if (!stralloc_copyb(&myfifo, "/tmp/", 5))
		die_nomem();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (!stralloc_cats(&myfifo, strnum))
		die_nomem();
	strnum[fmt_ulong(strnum, time(0))] = 0;
	if (!stralloc_cats(&myfifo, strnum) || !stralloc_0(&myfifo))
		die_nomem();
	rfd = wfd = -1;
	bytes = bytes - 7 + myfifo.len + sizeof(int); /*- "socket\0" */
	wfifo = prepare_wfifo(&wfd, &fifo_len, &pipe_size, bytes);
	if ((sig_pipe_save = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
		cleanup(rfd, wfd, 0, 0);
		strerr_warn1("tcplookup: signal: ", &strerr_sys);
		return (1);
	}
	if (!stralloc_ready(&querybuf, 2 + sizeof(int)))
		die_nomem();
	/*-
	 *  Format of Query Buffer
	 *  (len of string: int|QueryType: 1|NULL: 1|EmailId: len1|NULL: 1|Fifo: len2|NULL: 1|IP: len3|NULL: 1)
	 */
	ptr = querybuf.s;
	*((int *) ptr) = bytes - sizeof(int); /* size of query */
	ptr[sizeof(int)] = rquerybuf[0]; /*- query type */
	ptr[sizeof(int) + 1] = 0; /*- NULL */
	querybuf.len = sizeof(int) + 2;
	/*- email */
	if (!stralloc_cats(&querybuf, email) || !stralloc_0(&querybuf))
		die_nomem();
	if (!stralloc_cat(&querybuf, &myfifo))
		die_nomem();
	if (remoteip && *remoteip && !stralloc_cats(&querybuf, remoteip)) /*- ip */
		die_nomem();
	if (!stralloc_0(&querybuf))
		die_nomem();
	ptr = querybuf.s;
	if (FifoCreate(myfifo.s) == -1) {
		cleanup(-1, wfd, sig_pipe_save, 0);
		strerr_die3sys(111, "tcplookup: FifoCreate: ", myfifo.s, ": ");
	}
	if ((rfd = open(myfifo.s, O_RDONLY | O_NDELAY, 0)) == -1) {
		cleanup(-1, wfd, sig_pipe_save, myfifo.s);
		strerr_die3sys(111, "tcplookup: open: ", myfifo.s, ": ");
	}
	if (timeoutwrite(writeTimeout, wfd, ptr, bytes) != bytes) {
		cleanup(rfd, wfd, sig_pipe_save, myfifo.s);
		strerr_die3sys(111, "tcplookup: write: ", wfifo, ": ");
	}
	for (;;) {
		if ((idx = timeoutread(readTimeout, rfd, inbuf, sizeof(inbuf))) == -1) {
			idx = errno;
			cleanup(rfd, wfd, sig_pipe_save, myfifo.s);
			if (write(1, (char *) &idx, sizeof(int)) == -1)
				;
			errno = idx;
			strerr_die3sys(111, "tcplookup: read: ", myfifo.s, ": ");
		} else
		if (!idx) {
			close(1);
			break;
		}
		if (write(1, inbuf, idx) != idx) {
			cleanup(rfd, wfd, sig_pipe_save, myfifo.s);
			strerr_die1sys(111, "tcplookup: write: socket: ");
		}
	}
	cleanup(rfd, wfd, sig_pipe_save, myfifo.s);
	_exit (0);
}
