/*
 * $Log: inquery.c,v $
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
#include "r_mkdir.h"
#include "get_indimailuidgid.h"
#include "strToPw.h"

#ifndef	lint
static char     sccsid[] = "$Id: inquery.c,v 1.4 2020-10-18 07:49:06+05:30 Cprogrammer Exp mbhangui $";
#endif

/*- 
 *  Format of Query Buffer
 *  |len of string - int|QueryType - 1|NULL - 1|EmailId - len1|NULL - 1|Fifo - len2|NULL - 1|Ip - len3|NULL - 1|
 */
void           *
inquery(char query_type, char *email, char *ip)
{
	int             rfd, wfd, len, bytes, idx, readTimeout, writeTimeout,
					pipe_size, tmperrno, relative, fd;
	static int      intBuf, old_size = 0;
	char           *sysconfdir, *controldir, *infifo_dir, *infifo, *ptr;
	char            strnum[FMT_ULONG];
	static char    *pwbuf;
	void            (*sig_pipe_save) ();
	static stralloc querybuf = { 0 };
	static stralloc myfifo = { 0 };
	static stralloc InFifo = { 0 };
	static stralloc tmp = { 0 };

	userNotFound = 0;
	switch(query_type)
	{
		case RELAY_QUERY:
			if (!ip || !*ip)
			{
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
	if (!stralloc_copyb(&myfifo, "/tmp/", 5))
		return ((void *) 0);
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (!stralloc_cats(&myfifo, strnum))
		return ((void *) 0);
	strnum[fmt_ulong(strnum, time(0))] = 0;
	if (!stralloc_cats(&myfifo, strnum) || !stralloc_0(&myfifo))
		return ((void *) 0);
	if (!stralloc_cat(&querybuf, &myfifo)) /*- fifo */
		return ((void *) 0);
	if (ip && *ip) {
		if (!stralloc_cats(&querybuf, ip)) /*- ip */
			return ((void *) 0);
	}
	if (!stralloc_0(&querybuf))
		return ((void *) 0);
	ptr = querybuf.s;
	*((int *) ptr) = querybuf.len - sizeof(int);
	bytes = querybuf.len;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
	relative = *infifo_dir == '/' ? 0 : 1;
	if (!(infifo = env_get("INFIFO")))
		infifo = INFIFO;
	/*- Open the Fifos */
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&InFifo, infifo) || !stralloc_0(&InFifo))
			return ((void *) 0);
	} else {
		if (relative) {
			if (!stralloc_copys(&InFifo, INDIMAILDIR) ||
					!stralloc_cats(&InFifo, infifo_dir) ||
					!stralloc_catb(&InFifo, "/", 1) ||
					!stralloc_cats(&InFifo, infifo))
				return ((void *) 0);
		} else {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
			if (!stralloc_copys(&InFifo, infifo_dir) ||
					!stralloc_catb(&InFifo, "/", 1) ||
					!stralloc_cats(&InFifo, infifo))
				return ((void *) 0);
		}
		for (idx = 1, len = InFifo.len;;idx++) {
			InFifo.len = len;
			strnum[fmt_ulong(strnum, (unsigned long) idx)] = 0;
			if (!stralloc_catb(&InFifo, ".", 1) ||
					!stralloc_cats(&InFifo, strnum) ||
					!stralloc_0(&InFifo))
				return ((void *) 0);
			if (access(InFifo.s, F_OK))
				break;
		}
#ifdef RANDOM_BALANCING
		srand(getpid() + time(0));
#endif
		InFifo.len = len;
#ifdef RANDOM_BALANCING
		strnum[fmt_ulong(strnum, 1 + (int) ((float) (idx - 1) * rand()/(RAND_MAX + 1.0)))] = 0;
#else
		strnum[fmt_ulong(strnum, 1 + (time(0) % (idx - 1)))] = 0;
#endif
	}
	if (!stralloc_catb(&InFifo, ".", 1) ||
			!stralloc_cats(&InFifo, strnum) ||
			!stralloc_0(&InFifo))
		return ((void *) 0);
	if(verbose) {
		out("inquery", "Using INFIFO=");
		out("inquery", InFifo.s);
		out("inquery", "\n");
		flush("inquery");
	}
	if ((wfd = open(InFifo.s, O_WRONLY | O_NDELAY, 0)) == -1)
		return ((void *) 0);
	else 
	if (bytes > (pipe_size = fpathconf(wfd, _PC_PIPE_BUF))) {
		errno = EMSGSIZE;
		return ((void *) 0);
	} else
	if (FifoCreate(myfifo.s) == -1) {
		tmperrno = errno;
		close(wfd);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((rfd = open(myfifo.s, O_RDONLY | O_NDELAY, 0)) == -1) {
		tmperrno = errno;
		close(wfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((sig_pipe_save = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
		tmperrno = errno;
		close(rfd);
		close(wfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	}
	if (relative) {
		if (!stralloc_copys(&tmp, sysconfdir) ||
				!stralloc_catb(&tmp, "/", 1) ||
				!stralloc_cats(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/timeoutwrite", 13))
			return ((void *) 0);
	} else {
		if (!stralloc_copys(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/timeoutwrite", 13))
			return ((void *) 0);
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
	ptr = querybuf.s;
	if (timeoutwrite(writeTimeout, wfd, ptr, bytes) != bytes) {
		tmperrno = errno;
		signal(SIGPIPE, sig_pipe_save);
		close(wfd);
		close(rfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	}
	signal(SIGPIPE, sig_pipe_save);
	close(wfd);
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
			if (relative) {
				if (!stralloc_copys(&tmp, sysconfdir) ||
						!stralloc_catb(&tmp, "/", 1) ||
						!stralloc_cats(&tmp, controldir) ||
						!stralloc_catb(&tmp, "/timeoutread", 12))
					return ((void *) 0);
			} else {
				if (!stralloc_copys(&tmp, controldir) ||
						!stralloc_catb(&tmp, "/timeoutread", 12))
					return ((void *) 0);
			}
			if (!stralloc_0(&tmp))
				return ((void *) 0);
			if ((fd = open(tmp.s, O_RDONLY)) == -1)
				readTimeout = 4;
			else {
				if (read(fd, strnum, sizeof(strnum) - 1) == -1)
					readTimeout = 4;
				close(fd);
				scan_ulong(strnum, (unsigned long *) &readTimeout);
			}
			if ((idx = timeoutread(readTimeout, rfd, (char *) &intBuf, sizeof(int))) == -1 || !idx) {
				tmperrno = errno;
				close(rfd);
				unlink(myfifo.s);
				errno = tmperrno;
				return ((void *) 0);
			} else
			if (intBuf == -1) {
				close(rfd);
				unlink(myfifo.s);
				errno = 0;
				return ((void *) 0);
			} else
			if (intBuf > pipe_size) {
				close(rfd);
				unlink(myfifo.s);
				errno = EMSGSIZE;
				return ((void *) 0);
			}
			switch(query_type)
			{
				case USER_QUERY:
				case RELAY_QUERY:
					close(rfd);
					unlink(myfifo.s);
					return (&intBuf);
				case PWD_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
				case LIMIT_QUERY:
#endif
					if (!intBuf) {
						close(rfd);
						unlink(myfifo.s);
						errno = 0;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size && old_size)
						alloc_free(pwbuf);
					if (intBuf + 1 > old_size && !(pwbuf = alloc(intBuf + 1))) {
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size)
						old_size = intBuf + 1;
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx) {
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return ((void *) 0);
					}
					close(rfd);
					unlink(myfifo.s);
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
						close(rfd);
						unlink(myfifo.s);
						userNotFound = 1;
						errno = 0;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size && old_size)
						alloc_free(pwbuf);
					if (intBuf + 1 > old_size && !(pwbuf = alloc(intBuf + 1))) {
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return ((void *) 0);
					}
					if (intBuf + 1 > old_size)
						old_size = intBuf + 1;
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx) {
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return ((void *) 0);
					}
					close(rfd);
					unlink(myfifo.s);
					return (pwbuf);
					break;
				default:
					break;
			}
		default:
			break;
	} /*- switch(query_type) */
	close(rfd);
	unlink(myfifo.s);
	return ((void *) 0);
}
