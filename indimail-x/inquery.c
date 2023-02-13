/*
 * $Log: inquery.c,v $
 * Revision 1.9  2023-02-14 01:10:39+05:30  Cprogrammer
 * use FIFOTMPDIR instead of TMPDIR for inquery fifo
 *
 * Revision 1.8  2022-08-04 14:39:07+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.7  2022-07-31 10:06:35+05:30  Cprogrammer
 * use TMPDIR for /tmp
 *
 * Revision 1.6  2021-09-12 11:52:36+05:30  Cprogrammer
 * removed redundant multiple initialization of InFifo.len
 *
 * Revision 1.5  2021-02-07 19:55:54+05:30  Cprogrammer
 * make request over TCP/IP (tcpclient) using fd 6 and 7.
 *
 * Revision 1.4  2020-10-18 07:49:06+05:30  Cprogrammer
 * use alloc() instead of alloc_re()
 *
 * Revision 1.3  2020-04-01 18:55:52+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-22 23:12:30+05:30  Cprogrammer
 * replaced realloc() with alloc()
 *
 * Revision 1.1  2019-04-17 19:00:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <error.h>
#include <env.h>
#include <fmt.h>
#include <scan.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <getEnvConfig.h>
#endif
#include "FifoCreate.h"
#include "common.h"
#include "variables.h"
#include "strToPw.h"

#ifndef	lint
static char     sccsid[] = "$Id: inquery.c,v 1.9 2023-02-14 01:10:39+05:30 Cprogrammer Exp mbhangui $";
#endif

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

/*- 
 *  Format of Query Buffer
 *  (len of string: int|QueryType: 1|NULL: 1|EmailId: len1|NULL: 1|Fifo: len2|NULL: 1|IP: len3|NULL: 1)
 */
void           *
inquery(char query_type, char *email, char *ip)
{
	int             rfd, wfd, len, bytes, idx, readTimeout, writeTimeout,
					pipe_size, fd;
	static int      intBuf, old_size = 0;
	char           *sysconfdir, *controldir, *infifo_dir, *infifo, *ptr, *tcpclient;
	char            strnum[FMT_ULONG];
	static char    *pwbuf;
	void            (*sig_pipe_save) () = NULL;
	static stralloc querybuf = { 0 };
	static stralloc myfifo = { 0 };
	static stralloc InFifo = { 0 };
	static stralloc tmp = { 0 };

	userNotFound = 0;
	switch (query_type)
	{
		case RELAY_QUERY:
			if (!ip || !*ip) {
				errno = EINVAL;
				return ((void *) 0);
			}
		case USER_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			break;
		default:
			errno = EINVAL;
			return ((void *) 0);
	}
	/* - Prepare the Query Buffer for the Daemon */
	if (!stralloc_ready(&querybuf, 2 + sizeof(int)))
		return ((void *) 0);
	ptr = querybuf.s;
	ptr[sizeof(int)] = query_type; /*- query type */
	ptr[1 + sizeof(int)] = 0;
	querybuf.len = sizeof(int) + 2;
	/*- email */
	if (!stralloc_cats(&querybuf, email) || !stralloc_0(&querybuf))
		return ((void *) 0);
	if ((tcpclient = env_get("TCPCLIENT")))
		tcpclient = env_get("TCPREMOTEIP");
	if (!tcpclient) {
		if (!(ptr = env_get("FIFOTMPDIR")))
			ptr = "/tmp/";
		if (!stralloc_copys(&myfifo, ptr))
			return ((void *) 0);
		if (myfifo.s[myfifo.len -1] != '/' && !stralloc_append(&myfifo, "/"))
			return ((void *) 0);
		strnum[fmt_ulong(strnum, getpid())] = 0;
		if (!stralloc_cats(&myfifo, strnum))
			return ((void *) 0);
		strnum[fmt_ulong(strnum, time(0))] = 0;
		if (!stralloc_cats(&myfifo, strnum) || !stralloc_0(&myfifo))
			return ((void *) 0);
	} else
	if (!stralloc_copyb(&myfifo, "socket", 6) || !stralloc_0(&myfifo))
		return ((void *) 0);
	if (verbose) {
		out("inquery", "Using MYFIFO=");
		out("inquery", myfifo.s);
		out("inquery", "\n");
		flush("inquery");
	}
	if (!stralloc_cat(&querybuf, &myfifo) /*- fifo */ ||
			(ip && *ip && !stralloc_cats(&querybuf, ip)) /*- ip */ ||
			!stralloc_0(&querybuf))
		return ((void *) 0);
	ptr = querybuf.s;
	*((int *) ptr) = querybuf.len - sizeof(int); /*- datasize */
	/*-
	 * bytes = datasize + sizeof(int)
	 * i  = total length of data that needs to be transmitted to inlookup
	 */
	bytes = querybuf.len;

	if (!tcpclient) {
		if (!(infifo = env_get("INFIFO")))
			infifo = INFIFO; /*- the string "infifo" */
		/*- Open the Fifos */
		if (*infifo == '/' || *infifo == '.') {
			if (!stralloc_copys(&InFifo, infifo) || !stralloc_0(&InFifo))
				return ((void *) 0);
		} else {
			getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
			if (*infifo_dir == '/') {
				if (!stralloc_copys(&InFifo, infifo_dir) ||
						!stralloc_catb(&InFifo, "/", 1) ||
						!stralloc_cats(&InFifo, infifo))
					return ((void *) 0);
			} else {
				if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
						!stralloc_cats(&InFifo, infifo_dir) ||
						!stralloc_catb(&InFifo, "/", 1) ||
						!stralloc_cats(&InFifo, infifo))
					return ((void *) 0);
			}
			for (idx = 1, len = InFifo.len;;idx++) {
				strnum[fmt_ulong(strnum, (unsigned long) idx)] = 0;
				if (!stralloc_catb(&InFifo, ".", 1) ||
						!stralloc_cats(&InFifo, strnum) ||
						!stralloc_0(&InFifo))
					return ((void *) 0);
				InFifo.len = len;
				if (access(InFifo.s, F_OK))
					break;
			}
#ifdef RANDOM_BALANCING
			srand(getpid() + time(0));
			strnum[fmt_ulong(strnum, 1 + (int) ((float) (idx - 1) * rand()/(RAND_MAX + 1.0)))] = 0;
#else
			strnum[fmt_ulong(strnum, 1 + (time(0) % (idx - 1)))] = 0;
#endif
			if (!stralloc_catb(&InFifo, ".", 1) ||
					!stralloc_cats(&InFifo, strnum) || !stralloc_0(&InFifo))
				return ((void *) 0);
		}
	} /*- if (!tcpclient) { */
	if (verbose) {
		out("inquery", "Using INFIFO=");
		out("inquery", tcpclient ? tcpclient : InFifo.s);
		out("inquery", "\n");
		flush("inquery");
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/timeoutwrite", 13)) {
			return ((void *) 0);
		}
	} else {
		if (!stralloc_copys(&tmp, sysconfdir) ||
				!stralloc_catb(&tmp, "/", 1) ||
				!stralloc_cats(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/timeoutwrite", 13)) {
			return ((void *) 0);
		}
	}
	if (!stralloc_0(&tmp))
		return ((void *) 0);
	if ((fd = open(tmp.s, O_RDONLY)) == -1)
		writeTimeout = 4;
	else {
		if (read(fd, strnum, sizeof(strnum) - 1) == -1)
			writeTimeout = 4;
		close(fd);
		scan_ulong(strnum, (unsigned long *) &writeTimeout);
	}
	if (tcpclient) {
		rfd = 6;
		wfd = 7;
		pipe_size = 8192;
	} else {
		if ((wfd = open(InFifo.s, O_WRONLY | O_NDELAY, 0)) == -1)
			return ((void *) 0);
		else 
		if (bytes > (pipe_size = fpathconf(wfd, _PC_PIPE_BUF))) {
			errno = EMSGSIZE;
			return ((void *) 0);
		} else
		if (FifoCreate(myfifo.s) == -1) {
			cleanup(-1, wfd, 0, 0);
			return ((void *) 0);
		} else
		if ((rfd = open(myfifo.s, O_RDONLY | O_NDELAY, 0)) == -1) {
			cleanup(-1, wfd, 0, myfifo.s);
			return ((void *) 0);
		} else
		if ((sig_pipe_save = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
			cleanup(rfd, wfd, 0, myfifo.s);
			return ((void *) 0);
		}
	}
	ptr = querybuf.s;
	if (timeoutwrite(writeTimeout, wfd, ptr, bytes) != bytes) {
		if (!tcpclient)
			cleanup(rfd, wfd, sig_pipe_save, myfifo.s);
		return ((void *) 0);
	}
	if (!tcpclient) {
		signal(SIGPIPE, sig_pipe_save);
		close(wfd);
	}
	switch(query_type)
	{
		case USER_QUERY:
		case RELAY_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			tmp.len -= 6; /*- change timeoutwrite\0 to timeoutread\0 */
			if (!stralloc_catb(&tmp, "read", 4) || !stralloc_0(&tmp)) {
				if (!tcpclient)
					cleanup(rfd, -1, 0, myfifo.s);
				return ((void *) 0);
			}
			if ((fd = open(tmp.s, O_RDONLY)) == -1)
				readTimeout = 4;
			else {
				if (read(fd, strnum, sizeof(strnum) - 1) == -1)
					readTimeout = 4;
				close(fd);
				scan_ulong(strnum, (unsigned long *) &readTimeout);
			}
			/*- read an int to get size of data to be further read */
			if ((idx = timeoutread(readTimeout, rfd, (char *) &intBuf, sizeof(int))) == -1 || !idx) {
				if (!tcpclient)
					cleanup(rfd, -1, 0, myfifo.s);
				return ((void *) 0);
			} else
			if (intBuf == -1) { /*- system error on remote inlookup service */
				if (!tcpclient)
					cleanup(rfd, -1, 0, myfifo.s);
				errno = 0;
				return ((void *) 0);
			} else
			if (intBuf > pipe_size) {
				if (!tcpclient)
					cleanup(rfd, -1, 0, myfifo.s);
				errno = EMSGSIZE;
				return ((void *) 0);
			}
			switch(query_type)
			{
				case USER_QUERY:
				case RELAY_QUERY:
					if (!tcpclient) {
						close(rfd);
						unlink(myfifo.s);
					}
					return (&intBuf);
				case PWD_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
				case LIMIT_QUERY:
#endif
					if (!intBuf) { /*- error reading from remote inlookup */
						if (!tcpclient) {
							close(rfd);
							unlink(myfifo.s);
						}
						errno = 0;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size && old_size)
						alloc_free(pwbuf);
					if (intBuf + 1 > old_size && !(pwbuf = alloc(intBuf + 1))) {
						if (!tcpclient)
							cleanup(rfd, -1, 0, myfifo.s);
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size)
						old_size = intBuf + 1;
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx) {
						if (!tcpclient)
							cleanup(rfd, -1, 0, myfifo.s);
						return ((void *) 0);
					}
					if (!tcpclient) {
						close(rfd);
						unlink(myfifo.s);
					}
#ifdef ENABLE_DOMAIN_LIMITS
					if (query_type == PWD_QUERY)
						return (strToPw(pwbuf, intBuf));
					else
						return (pwbuf);
#else
					return (strToPw(pwbuf, intBuf));
#endif
				case ALIAS_QUERY:
#ifdef CLUSTERED_SITE
				case HOST_QUERY:
#endif
				case DOMAIN_QUERY:
					if (!intBuf) {
						if (!tcpclient) {
							close(rfd);
							unlink(myfifo.s);
						}
						userNotFound = 1;
						errno = 0;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size && old_size)
						alloc_free(pwbuf);
					if (intBuf + 1 > old_size && !(pwbuf = alloc(intBuf + 1))) {
						if (!tcpclient)
							cleanup(rfd, -1, 0, myfifo.s);
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size)
						old_size = intBuf + 1;
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx) {
						if (!tcpclient)
							cleanup(rfd, -1, 0, myfifo.s);
						return ((void *) 0);
					}
					if (!tcpclient) {
						close(rfd);
						unlink(myfifo.s);
					}
					return (pwbuf);
					break;
				default:
					break;
			}
		default:
			break;
	} /*- switch(query_type) */
	if (!tcpclient) {
		close(rfd);
		unlink(myfifo.s);
	}
	return ((void *) 0);
}
