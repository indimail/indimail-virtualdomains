#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef SOLARIS
#include <sys/systeminfo.h>
#endif
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include <tls.h>
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <subfd.h>
#include <sgetopt.h>
#include <getln.h>
#include <sig.h>
#include <alloc.h>
#include <strerr.h>
#include <tcpopen.h>
#include <open.h>
#include <qprintf.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <error.h>
#include <env.h>
#include <getEnvConfig.h>
#include <qmail/tls.h>
#endif

#define SEEKDIR PREFIX"/tmp/"
#define WARN    "logclient: warn: " 
#define FATAL   "logclient: fatal: " 

#ifndef	lint
static char     sccsid[] = "$Id: logclient.c,v 1.16 2024-05-31 09:01:34+05:30 Cprogrammer Exp mbhangui $";
#endif


struct msgtab
{
	char           *fn;
	ino_t           inum;
	int             fd;
	int             seekfd;
	off_t           seek;
	long            count;
};
stralloc        seekfile = {0};
struct msgtab  *msghd;
char           *lhost, *seekdir = SEEKDIR;
unsigned int    port = 6340;
int             exitasap = 0, sleepinterval = 5, verbose;
#ifdef HAVE_SSL
unsigned long   ctimeout = 60;
#endif
unsigned long   dtimeout = 300;
#ifdef SERVER
int             server_mode = 1;
#endif
static char    *usage =
	"usage: logclient [options] loghost logfile1 [logfile2 ...]\n"
	"options\n"
	"  -l localhost      - local hostname\n"
	"  -d dtimeout       - data timeout\n"
#ifdef HAVE_SSL
	"  -c ctimeout       - connect timeout\n"
	"  -n certfile       - openssl certificate\n"
	"  -c cafile         - openssl ca certificate\n"
	"  -r crlfile        - openssl revocation list\n"
#endif
	"  -v                - verbose output";

static void
sigterm(int x)
{
	exitasap = 1;
}

static int
checkfiles(struct msgtab *msgptr)
{
	int             fd, count;
	long            seekval[1];
	struct stat     st;

	for (count = 0;; count++) {
		if (access(msgptr->fn, F_OK)) {
			if (errno == error_noent) {
				sleep(5);
				continue;
			}
			strerr_warn3(WARN, msgptr->fn, ": ", &strerr_sys);
			return -1;
		} else
		if (count) { /* new file has been created */
			close(msgptr->fd);
			if ((fd = open_read(msgptr->fn)) == -1) {
				strerr_warn3(WARN, msgptr->fn, ": ", &strerr_sys);
				return -1;
			}
			if (dup2(fd, msgptr->fd) == -1) {
				strerr_warn3(WARN, msgptr->fn, ": dup2: ", &strerr_sys);
				return -1;
			}
			if (fd != msgptr->fd)
				close(fd);
			if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1) {
				strerr_warn4(WARN, "unable to rewind ", msgptr->fn, ".seek: ", &strerr_sys);
				return -1;
			}
			seekval[0] = 0l;
			if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1) {
				strerr_warn4(WARN, "unable to write to ", msgptr->fn, ".seek: ", &strerr_sys);
				return -1;
			}
			return 1; /*- new file got created */
		} else
			break;
	}
	if ((fd = open_read(msgptr->fn)) == -1) {
		strerr_warn4(WARN, "open: ", msgptr->fn, ": ", &strerr_sys);
		return -1;
	}
	if (fstat(fd, &st) == -1) {
		strerr_warn4(WARN, "fstat: ", msgptr->fn, ": ", &strerr_sys);
		return -1;
	}
	if (st.st_ino == msgptr->inum) {
		if (st.st_size == msgptr->seek)/* Just an EOF on file */ {
			close(fd);
			return 0;
		} else
		if (st.st_size > msgptr->seek) { /* update happened after EOF */
			close(fd);
			return 2; /*- original file got updated */
		} else { /*- file got truncated */
			if (lseek(msgptr->fd, 0l, SEEK_SET) == -1) {
				strerr_warn4(WARN, "unable to rewind ", msgptr->fn, ": ", &strerr_sys);
				return -1;
			}
			if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1) {
				strerr_warn4(WARN, "unable to rewind ", msgptr->fn, ".seek: ", &strerr_sys);
				return -1;
			}
			seekval[0] = 0l;
			if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1) {
				strerr_warn4(WARN, "unable to write to ", msgptr->fn, ".seek: ", &strerr_sys);
				return -1;
			}
			return 1;
		}
	} else { /* new file has been created */
		msgptr->inum = st.st_ino;
		close(msgptr->fd);
		if (dup2(fd, msgptr->fd) == -1) {
			strerr_warn3(WARN, msgptr->fn, ": dup2: ", &strerr_sys);
			return -1;
		}
		if (fd != msgptr->fd)
			close(fd);
		if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1) {
			strerr_warn4(WARN, "unable to rewind ", msgptr->fn, ".seek: ", &strerr_sys);
			return -1;
		}
		seekval[0] = 0l;
		if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1) {
			strerr_warn4(WARN, "unable to write to ", msgptr->fn, ".seek: ", &strerr_sys);
			return -1;
		}
		return 1;
	}
}

/* function for I/O multiplexing */
static int
transmit_logs(char *lhost, char *remote, int sfd)
{
	int             n, i, match, flag;
	fd_set          FdSet;
	struct stat     st;
	long            seekval[2];
	struct msgtab  *msgptr;
	substdio        ssin;
	char            inbuf[512], ack[1];
	static stralloc line = {0}, buf = {0};
	struct timeval  tm;

	for (;!exitasap;) {
		/* include message files and sfd in file descriptor set */
		FD_ZERO(&FdSet);
		for (msgptr = msghd; !exitasap && msgptr->fd != -1; msgptr++) {
			if (fstat(msgptr->fd, &st) == -1) {
				strerr_warn4(WARN, "fstat: ", msgptr->fn, ": ", &strerr_sys);
				break;
			}
			if (st.st_size != msgptr->seek)
				FD_SET(msgptr->fd, &FdSet);
		}
		FD_SET(sfd, &FdSet);
		tm.tv_sec = sleepinterval;
		tm.tv_usec = 0;
		if ((n = select(sfd + 1, &FdSet, (fd_set *) 0, (fd_set *) 0, &tm)) == -1) {
			if (errno == EINTR)
				continue;
			strerr_warn2(WARN, "select: ", &strerr_sys);
#ifdef HAVE_SSL
			ssl_free();
#endif
			break;
		}
		if (!n) /*- timeout */
			continue;
		if (FD_ISSET(sfd, &FdSet)) {
			for (;!exitasap;) {
				errno = 0;
				if ((n = tlsread(sfd, inbuf, sizeof(inbuf), dtimeout)) == -1) {
					if (errno == EINTR)
						continue;
					return -1;
				}
				break;
			}
			if (!n)
				return 0;
		}
		for (msgptr = msghd; !exitasap && msgptr->fd != -1; msgptr++) {
			if (FD_ISSET(msgptr->fd, &FdSet))
				break;
		}
		for (msgptr = msghd; !exitasap && msgptr->fd != -1; msgptr++) {
			if (!FD_ISSET(msgptr->fd, &FdSet))
				continue;
			substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, msgptr->fd, inbuf, sizeof(inbuf));
			if (msgptr->seek) {
				if (lseek(msgptr->fd, msgptr->seek, SEEK_SET) == -1) {
					strerr_warn4(WARN, "unable to seek ", msgptr->fn, ": ", &strerr_sys);
					return -1;
				}
			}
			for (flag = 0; !exitasap;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn4(WARN, "read: ", msgptr->fn, ": ", &strerr_sys);
					return -1;
				}
				if (!line.len) {
					i = checkfiles(msgptr);
					switch (i)
					{
					case 0:
						FD_CLR(msgptr->fd, &FdSet);
						break;
					case 1: /*- new file got created, truncated */
						msgptr->seek = 0;
						continue;
					case 2: /*- original file got updated */
						ssin.p = 0;
						continue;
					}
					break;
				}
				if (!stralloc_0(&line)) {
					strerr_warn2(WARN, "out of memory", 0);
					break;
				}
				line.len--;
				qsprintf(&buf, "%s %ld %s", msgptr->fn, msgptr->count, line.s);
				if (tlswrite(sfd, buf.s, buf.len - 1, dtimeout) == -1) {
					strerr_warn4(WARN, "unable to transmit log to host ", remote, ": ", &strerr_sys);
					break;
				}
				if (verbose) {
					subprintf(subfdout, "%s", buf.s);
					substdio_flush(subfdout);
				}
				if (FD_ISSET(sfd, &FdSet)) {
					subprintf(subfdout, "sfd is available for read\n");
					substdio_flush(subfdout);
				}
				for (;!exitasap;) {
					if ((n = tlsread(sfd, ack, sizeof(ack), dtimeout)) == -1) {
						if (errno == EINTR)
							continue;
						strerr_warn4(WARN, "unable to read ack from host ", remote, ": ", &strerr_sys);
						ack[0] = '0';
					} else
					if (!n) {
						strerr_warn4(WARN, "host ", remote, " closed connection: ", &strerr_sys);
						ack[0] = '0';
					}
					break;
				}
				if (ack[0] != '1') {
					strerr_warn4(WARN, "host ", remote, " acknowledged error writing log", 0);
					flag = 0;
					break;
				}
				flag = 1;
				msgptr->count++;
				if ((msgptr->seek = lseek(msgptr->fd, (off_t) 0, SEEK_CUR)) == -1) {
					strerr_warn4(WARN, "unable to get current position ", msgptr->fn, ": ", &strerr_sys);
					return -1;
				}
			} /* end of for (flag = 0;;) */
			if (flag) { /*- some data was transmitted to loghost */
				if (lseek(msgptr->seekfd, 0, SEEK_SET) == -1) {
					strerr_warn4(WARN, "unable to rewind ", msgptr->fn, ".seek: ", &strerr_sys);
					return -1;
				}
				seekval[0] = msgptr->seek;
				seekval[1] = msgptr->count;
				if (write(msgptr->seekfd, (char *) seekval, 2 * sizeof(long)) == -1)
					strerr_warn4(WARN, "unable to write to ", msgptr->fn, ".seek:", &strerr_sys);
			}
		} /* end of for (msgptr = msghd;;) */
	} /* end of for (;;) */
	_exit (0);
}

int
consclnt(char *remote, char **argv, char *clientcert, char *cafile, char *crlfile, int match_cn)
{
	int             sfd, idx;
	long            seekval[2];
	struct msgtab  *msgptr;
	struct stat     st;
	char          **fptr;
#ifdef HAVE_SSL
	SSL            *ssl;
	SSL_CTX        *ctx;
	char           *ciphers;
#endif

#ifdef SERVER
	if (server_mode) {
		switch (fork())
		{
		case -1:
			strerr_die2sys(111, FATAL, "fork: ");
			return 1;
		case 0:
			close(0);
			close(1);
			close(2);
			setsid();
			break;
		default:
			return 0;
		}
	}
#endif
	for (fptr = argv, msgptr = msghd; *fptr; msgptr++, fptr++) {
		msgptr->fn = *fptr;
		if (stat(msgptr->fn, &st) == -1)
			strerr_die4sys(111, FATAL, "stat: ", msgptr->fn, ": ");
		msgptr->inum = st.st_ino;
		if (qsprintf(&seekfile, "%s/%lu.seek", seekdir, msgptr->inum) == -1)
			strerr_die2x(111, FATAL, "out of memory");
		if ((msgptr->fd = open_read(msgptr->fn)) == -1)
			strerr_die4sys(111, FATAL, "open: ", msgptr->fn, ": ");
		if (!access(seekfile.s, R_OK)) {
			if ((msgptr->seekfd = open_readwrite(seekfile.s)) == -1)
				strerr_die4sys(111, FATAL, "open: ", seekfile.s, ": ");
			if (read(msgptr->seekfd, (char *) seekval, 2 * sizeof(long)) == -1)
				strerr_die4sys(111, FATAL, "read: ", seekfile.s, ": ");
			msgptr->seek = seekval[0];
			msgptr->count = seekval[1];
		} else
		if ((msgptr->seekfd = open(seekfile.s, O_CREAT | O_RDWR, 0644)) == -1)
			strerr_die4sys(111, FATAL, "open read+write: ", seekfile.s, ": ");
		else {
			msgptr->seek = 0l;
			msgptr->count = 0l;
		}
	} /* for (fptr = argv, msgptr = msghd; *fptr; msgptr++, fptr++) */
	msgptr->fn = NULL;
	msgptr->fd = -1;
	msgptr->seekfd = -1;
#ifdef DEBUG
	for (msgptr = msghd;msgptr->fd != -1;msgptr++) {
		subprintf(subfderr, "filename: %s\n", msgptr->fn);
		subprintf(subfderr, "file  fd: %d\n", msgptr->fd);
		subprintf(subfderr, "seek  fd: %d\n", msgptr->seekfd);
		subprintf(subfderr, "seek val: %ld\n", msgptr->seek);
		subprintf(subfderr, "count   : %ld\n", msgptr->count);
		substdio_flush(subfderr);
	}
#endif
	for (;;sleep(5)) {
		for (;;) {
			if ((sfd = tcpopen(remote, "logsrv", port)) == -1) {
				if (errno == EINVAL)
					return 1;
				sleep(5);
				continue;
			}
			break;
		}
#ifdef HAVE_SSL
		if (clientcert) {
			if (!(ciphers = env_get("TLS_CIPHER_LIST")))
				ciphers = "PROFILE=SYSTEM";
			if (!(ctx = tls_init(0, clientcert, cafile, crlfile, ciphers, client)))
				return -1;
			if (!(ssl = tls_session(ctx, sfd))) {
				SSL_CTX_free(ctx);
				return -1;
			}
			SSL_CTX_free(ctx);
			ctx = NULL;
			if (tls_connect(ctimeout, sfd, sfd, ssl, match_cn ? remote : 0) == -1)
				return -1;
		}
#endif
		/*- send my hostname and a null byte */
		idx = str_len(lhost) + 1;
		if (tlswrite(sfd, lhost, idx, dtimeout) != idx) {
			close(sfd);
			strerr_warn4(WARN, "unable to send hostname to ", remote, ": ", &strerr_sys);
#ifdef HAVE_SSL
			ssl_free();
#endif
		}
		transmit_logs(lhost, remote, sfd);
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
	} /* for (;;) */
}

int
main(int argc, char **argv)
{
#ifdef HOSTVALIDATE	
	struct hostent *hostptr;
#endif
	int             c, match_cn = 0;
	char           *remote, *certfile = NULL, *cafile = NULL,
				   *crlfile = NULL;
	char            optstr[22];

	c = 0;
	c += fmt_strn(optstr + c, "vl:s:i:d:p:", 11);
#ifdef SERVER
	c += fmt_strn(optstr + c, "f", 1);
#endif
#ifdef HAVE_SSL
	c += fmt_strn(optstr + c, "c:n:C:r:m", 9);
#endif
	if ((c + 1) > sizeof(optstr))
		strerr_die2x(100, FATAL, "allocated space for getopt string not enough");
	sig_pipeignore();
	sig_childdefault();
	sig_termcatch(sigterm);
	sig_intcatch(sigterm);
	optstr[c] = 0;
	while ((c = getopt(argc, argv, optstr)) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'l':
			lhost = optarg;
			break;
		case 's':
			seekdir = optarg;
			break;
		case 'i':
			scan_int(optarg, &sleepinterval);
			break;
		case 'd':
			scan_ulong(optarg, &dtimeout);
			break;
		case 'p':
			scan_uint(optarg, &port);
			break;
#ifdef SERVER
		case 'f':
			server_mode = 0;
			break;
#endif
#ifdef HAVE_SSL
		case 'c':
			scan_ulong(optarg, &ctimeout);
			break;
		case 'n':
			certfile = optarg;
			break;
		case 'C':
			cafile = optarg;
			break;
		case 'r':
			crlfile = optarg;
			break;
		case 'm':
			match_cn = 1;
			break;
#endif
		default:
			strerr_die1x(100, usage);
			break;
		}
	}
	if (!server_mode)
		sig_intcatch(sigterm);
	if (!lhost || (argc - optind) < 2)
		strerr_die1x(100, usage);
	remote = argv[optind++];
#ifdef HOSTVALIDATE	
	if (!(hostptr = gethostbyname(remote)))
		strerr_die2x(100, remote, " not found in /etc/hosts");
	endhostent();
#endif	
	if (!(msghd = (struct msgtab *) alloc(sizeof(struct msgtab) * (argc - optind + 1)))) {
		strerr_warn2(WARN, "out of memory", 0);
		return 1;
	}
	return (consclnt(remote, argv + optind, certfile, cafile, crlfile, match_cn));
}

#ifndef	lint
#include <stdio.h>
void
getversion_logclient_c()
{
	printf("%s\n", sccsid);
}
#endif

/*
 * $Log: logclient.c,v $
 * Revision 1.16  2024-05-31 09:01:34+05:30  Cprogrammer
 * replaced perror with strerr_die2sys
 *
 * Revision 1.15  2023-10-15 18:10:43+05:30  Cprogrammer
 * ensure qmail/tls.h gets included
 *
 * Revision 1.14  2023-04-14 22:30:27+05:30  Cprogrammer
 * added -v, -p option
 *
 * Revision 1.13  2023-04-14 09:42:08+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.12  2022-12-06 12:05:21+05:30  Cprogrammer
 * removed filewrt
 *
 * Revision 1.11  2022-05-10 20:09:57+05:30  Cprogrammer
 * use tcpopen from libqmail
 *
 * Revision 1.10  2021-04-05 21:57:58+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.9  2021-03-15 11:32:07+05:30  Cprogrammer
 * removed common.h
 *
 * Revision 1.8  2020-06-21 12:49:01+05:30  Cprogrammer
 * quench rpmlint
 *
 * Revision 1.7  2013-05-15 00:38:47+05:30  Cprogrammer
 * added SSL encryption
 *
 * Revision 1.6  2013-02-21 22:45:39+05:30  Cprogrammer
 * fold long line for readability
 *
 * Revision 1.5  2012-09-19 11:06:49+05:30  Cprogrammer
 * use environment variable SEEKDIR for the checkpoint files
 *
 * Revision 1.4  2012-09-18 17:39:43+05:30  Cprogrammer
 * fixed usage
 *
 * Revision 1.3  2012-09-18 17:29:34+05:30  Cprogrammer
 * use sockfd instead of sockfp
 *
 * Revision 1.2  2012-09-18 17:08:34+05:30  Cprogrammer
 * removed extra white space
 *
 * Revision 1.1  2012-09-18 13:23:43+05:30  Cprogrammer
 * Initial revision
 *
 */
