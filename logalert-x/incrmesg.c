/*
 * $Log: incrmesg.c,v $
 * Revision 1.11  2023-04-14 00:08:17+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.10  2022-05-10 20:09:47+05:30  Cprogrammer
 * use tcpopen from libqmail
 *
 * Revision 1.9  2022-05-10 01:11:02+05:30  Cprogrammer
 * include r_mkdir.h
 *
 * Revision 1.8  2021-04-05 21:57:22+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.7  2021-03-15 11:31:57+05:30  Cprogrammer
 * removed common.h
 *
 * Revision 1.6  2020-06-21 12:47:48+05:30  Cprogrammer
 * quench rpmlint
 *
 * Revision 1.5  2013-05-15 00:19:32+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.4  2013-03-03 23:36:12+05:30  Cprogrammer
 * create the directory with owner of incrmesg process
 *
 * Revision 1.3  2012-12-13 08:36:45+05:30  Cprogrammer
 * use SEEKDIR env variable
 *
 * Revision 1.2  2012-09-18 17:39:28+05:30  Cprogrammer
 * changed seek directory to /var/tmp
 *
 * Revision 1.1  2012-09-18 13:26:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <getln.h>
#include <subfd.h>
#include <qprintf.h>
#include <sgetopt.h>
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include <error.h>
#include <strerr.h>
#include <open.h>
#include <r_mkdir.h>
#include <alloc.h>
#endif

#define SEEKDIR "/var/tmp/incrmesg"
#define WARN    "incrmesg: warn: " 
#define FATAL   "incrmesg: fatal: " 

#ifndef	lint
static char     rcsid[] = "$Id: incrmesg.c,v 1.11 2023-04-14 00:08:17+05:30 Cprogrammer Exp mbhangui $";
#endif

struct msgtab
{
	char           *fname;
	ino_t           inum; 
	int             fd;
	int             seekfd;
	long            seek;
	long            count;
};

char           *lhost;
struct msgtab  *msghd;
stralloc        seekfile = {0};

static int
checkfiles(struct msgtab *msgptr)
{
	int             fd, count;
	long            seekval[1];
	struct stat     st;

	for (count = 0;; count++) {
		if (access(msgptr->fname, F_OK)) {
			if (errno == error_noent) {
				sleep(5);
				continue;
			}
			strerr_die3sys(111, FATAL, msgptr->fname, ": ");
		} else
		if (count) { /* new file has been created */
			close(msgptr->fd);
			if ((fd = open_read(msgptr->fname)) == -1)
				strerr_die3sys(111, FATAL, msgptr->fname, ": ");
			if (dup2(fd, msgptr->fd) == -1)
				strerr_die3sys(111, FATAL, msgptr->fname, ": dup2: ");
			if (fd != msgptr->fd)
				close(fd);
			if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1)
				strerr_die4sys(111, FATAL, "unable to rewind ", msgptr->fname, ".seek: ");
			seekval[0] = 0l;
			if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1)
				strerr_die4sys(111, FATAL, "unable to write to ", msgptr->fname, ".seek: ");
			return (1); /*- new file got created */
		} else
			break;
	}
	if ((fd = open_read(msgptr->fname)) == -1)
		strerr_die4sys(111, FATAL, "open: ", msgptr->fname, ": ");
	if (fstat(fd, &st) == -1)
		strerr_die4sys(111, FATAL, "fstat: ", msgptr->fname, ": ");
	if (st.st_ino == msgptr->inum) {
		if (st.st_size == msgptr->seek)/* Just an EOF on file */ {
			close(fd);
			return (0);
		} else
		if (st.st_size > msgptr->seek) { /* update happened after EOF */
			close(fd);
			return (2); /*- original file got updated */
		} else { /*- file got truncated */
			if (lseek(msgptr->fd, 0l, SEEK_SET) == -1)
				strerr_die4sys(111, FATAL, "unable to rewind ", msgptr->fname, ": ");
			if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1)
				strerr_die4sys(111, FATAL, "unable to rewind ", msgptr->fname, ".seek: ");
			seekval[0] = 0l;
			if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1)
				strerr_die4sys(111, FATAL, "unable to write to ", msgptr->fname, ".seek: ");
			return (1);
		}
	} else { /* new file has been created */
		msgptr->inum = st.st_ino;
		close(msgptr->fd);
		if (dup2(fd, msgptr->fd) == -1)
			strerr_die3sys(111, FATAL, msgptr->fname, ": dup2: ");
		if (fd != msgptr->fd)
			close(fd);
		if (lseek(msgptr->seekfd, 0l, SEEK_SET) == -1)
			strerr_die4sys(111, FATAL, "unable to rewind ", msgptr->fname, ".seek: ");
		seekval[0] = 0l;
		if (write(msgptr->seekfd, (char *) seekval, sizeof(long)) == -1)
			strerr_die4sys(111, FATAL, "unable to write to ", msgptr->fname, ".seek: ");
		return (1);
	}
}

static int
IOplex()
{
	long            seekval[2], startsrno;
	int             dflag, match, i;
	struct msgtab  *msgptr;
	substdio        ssin;
	char            inbuf[512];
	static stralloc line = {0};

	for (msgptr = msghd; msgptr->fd != -1; msgptr++) {
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, msgptr->fd, inbuf, sizeof(inbuf));
		if (msgptr->seek) {
			if (lseek(msgptr->fd, msgptr->seek, SEEK_SET) == -1)
				strerr_die4sys(111, FATAL, "unable to seek ", msgptr->fname, ": ");
		}
		for (dflag = 0, startsrno = msgptr->count;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				strerr_die4sys(111, FATAL, "read: ", msgptr->fname, ": ");
			if (!line.len) {
				i = checkfiles(msgptr);
				switch (i)
				{
				case 1:
					continue;
				case 2:
					ssin.p = 0;
					continue;
				}
				if (dflag) {
					if (subprintf(subfdout, "=======================================\n") == -1 ||
							subprintf(subfdout, "Message        File : %s@%s\n", msgptr->fname, lhost) == -1 ||
							subprintf(subfdout, "Message       Count : %ld\n", msgptr->count - startsrno) == -1 ||
							subprintf(subfdout, "Start Serial Number : %ld\n", startsrno) == -1 ||
							subprintf(subfdout, "End   Serial Number : %ld\n", msgptr->count - 1) == -1 ||
							subprintf(subfdout, "=======================================\n")) {
						strerr_warn2(WARN, "unable to write to descriptor 1: ", &strerr_sys);
					}
				}
				break;
			}
			if (!stralloc_0(&line))
				strerr_die2x(111, FATAL, "out of memory");
			if (!dflag++) {
				if (subprintf(subfdout, "Filename %-25s\n", msgptr->fname) == -1 ||
						subprintf(subfdout, "=======================================================\n") == -1) {
					strerr_warn2(WARN, "unable to write to descriptor 1: ", &strerr_sys);
				}
			}
			if (lhost) {
				if (subprintf(subfdout, "%s %ld %s", lhost, (msgptr->count)++, line.s) == -1) {
					strerr_warn2(WARN, "unable to write to descriptor 1: ", &strerr_sys);
					break;
				}
			} else {
				if (subprintf(subfdout, "%ld %s", (msgptr->count)++, line.s) == -1) {
					strerr_warn2(WARN, "unable to write to descriptor 1: ", &strerr_sys);
					break;
				}
			}
			if ((msgptr->seek = lseek(msgptr->fd, (off_t) 0, SEEK_CUR)) == -1)
				strerr_die4sys(111, FATAL, "unable to get current position ", msgptr->fname, ": ");
		} /* end of for (dflag = 0;;) */
		if (dflag) {
			if (substdio_flush(subfdout) == -1) {
				strerr_warn2(WARN, "unable to write to descriptor 1: ", &strerr_sys);
				continue;
			}
			if (lseek(msgptr->seekfd, 0, SEEK_SET) == -1)
				strerr_die4sys(111, FATAL, "unable to rewind ", msgptr->fname, ".seek: ");
			seekval[0] = msgptr->seek;
			seekval[1] = msgptr->count;
			if (write(msgptr->seekfd, (char *) seekval, 2 * sizeof(long)) == -1)
				strerr_warn4(WARN, "unable to write to ", msgptr->fname, ".seek:", &strerr_sys);
			msgptr->seek = seekval[0];
		}
		close(msgptr->seekfd);
		close(msgptr->fd);
	} /* end of for (msgptr = msghd;;) */
	return (0);
}

int
incrmesg(char **argv)
{
	long            seekval[2];
	struct msgtab  *msgptr;
	char           *seekdir;
	char          **fptr;
	struct stat     st;

	if (!(seekdir = env_get("SEEKDIR")))
		seekdir = SEEKDIR;
	if (access(seekdir, F_OK) && r_mkdir(seekdir, 0700, getuid(), getgid()))
		strerr_die4sys(111, FATAL, "r_mkdir: ", seekdir, ": ");
	for (fptr = argv, msgptr = msghd; *fptr; msgptr++, fptr++) {
		msgptr->fname = *fptr;
		if (stat(msgptr->fname, &st) == -1)
			strerr_die4sys(111, FATAL, "stat: ", msgptr->fname, ": ");
		msgptr->inum = st.st_ino;

		if (qsprintf(&seekfile, "%s/%ld.seek", seekdir, msgptr->inum) == -1)
			strerr_die2x(111, FATAL, "out of memory");
		if ((msgptr->fd = open_read(msgptr->fname)) == -1)
			strerr_die4sys(111, FATAL, "open: ", msgptr->fname, ": ");
		if (!access(seekfile.s, R_OK)) {
			if ((msgptr->seekfd = open_readwrite(seekfile.s)) == -1)
				strerr_die4sys(111, FATAL, "open: ", seekfile.s, ": ");
			if (read(msgptr->seekfd, (char *) seekval, 2 * sizeof(long)) == -1)
				strerr_die4sys(111, FATAL, "read1: ", seekfile.s, ": ");
			msgptr->seek = seekval[0];
			msgptr->count = seekval[1];
		} else
		if ((msgptr->seekfd = open(seekfile.s, O_CREAT | O_RDWR, 0644)) == -1)
			strerr_die4sys(111, FATAL, "open read+write: ", seekfile.s, ": ");
		else {
			msgptr->seek = 0l;
			msgptr->count = 0l;
		}
	}
	msgptr->fname = NULL;
	msgptr->fd = -1;
	msgptr->seekfd = -1;
#ifdef DEBUG
	for (msgptr = msghd;msgptr->fd != -1;msgptr++) {
		subprintf(subfderr, "filename: %s\n", msgptr->fname);
		subprintf(subfderr, "file  fd: %d\n", msgptr->fd);
		subprintf(subfderr, "seek  fd: %d\n", msgptr->seekfd);
		subprintf(subfderr, "seek val: %ld\n", msgptr->seek);
		subprintf(subfderr, "count   : %ld\n", msgptr->count);
		substdio_flush(subfderr);
	}
#endif
	return (IOplex());
}

int
main(int argc, char **argv)
{
	int             c;

	while ((c = getopt(argc, argv, "l:")) != opteof) {
		switch (c)
		{
		case 'l':
			lhost = optarg;
			break;
		default:
			strerr_die2x(100, WARN, "USAGE: incrmesg logfile(s)");
		}
	}
	if (argc == optind)
		strerr_die2x(100, WARN, "USAGE: incrmesg logfile(s)");
	if (!(msghd = (struct msgtab *) alloc(sizeof(struct msgtab) * argc)))
		strerr_die2x(111, FATAL, "out of memory");
	return (incrmesg(argv + optind));
}

#ifndef	lint
void
getversion_incrmesg_c()
{
	char *x;
	x = rcsid;
	x++;
}
#endif
