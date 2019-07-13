/*
 * $Log: remove_line.c,v $
 * Revision 1.1  2019-04-18 08:36:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <open.h>
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#endif
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: remove_line.c,v 1.1 2019-04-18 08:36:17+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("remove_line: out of memory", 0);
	_exit(111);
}

/*
 * Generic remove a line from a file utility
 * input: template to search for
 *        file to search inside
 *
 * output: less than zero on failure
 *         0 if successful
 *         1 if match found
 */
int
remove_line(char *template, char *filename, int once_only, mode_t mode)
{
	struct substdio ssin, ssout;
	static stralloc fname = {0}, line = {0};
	struct stat     statbuf;
	char            inbuf[4096], outbuf[4096];
	int             fd1, fd2, match, found;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	if (stat(filename, &statbuf)) {
		strerr_warn3("remove_line: stat: ", filename, ": ", &strerr_sys);
		return (-1);
	}
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(filename, 1)) == -1) {
		strerr_warn3("remove_line: getDbLock: ", filename, ": ", &strerr_sys);
		return (-1);
	} 
#endif
	/*- format a new string */
	if (!stralloc_copys(&fname, filename)
			|| !stralloc_catb(&fname, ".bak", 4) || !stralloc_0(&fname))
		die_nomem();
	if ((fd2 = open(fname.s, O_CREAT|O_TRUNC|O_WRONLY, mode)) == -1) {
		strerr_warn3("remove_line: open: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		return (-1);
	}
	if (fchmod(fd2, mode) || fchown(fd2, statbuf.st_uid, statbuf.st_gid)) {
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		strerr_warn3("remove_line: fchown/fchmod: ", fname.s, ": ", &strerr_sys);
		unlink(fname.s);
		return (-1);
	}
	/*- open in read mode and check for error */
	if ((fd1 = open_read(filename)) == -1) {
		strerr_warn3("remove_line: open: ", filename, ": ", &strerr_sys);
		close(fd2);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		unlink(fname.s);
	}
	substdio_fdbuf(&ssin, read, fd1, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, fd2, outbuf, sizeof(outbuf));
	/*- pound away on the files run the search algorithm */
	for (found = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("remove_line: read: ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(lockfd, filename, 1);
#endif
			close(fd1);
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
		if (!match && line.len == 0)
			break;
		if (found && once_only) {
			if (substdio_put(&ssout, line.s, line.len)) {
				strerr_warn3("remove_line: write error: ", fname.s, ": ", &strerr_sys);
				close(fd2);
				unlink(fname.s);
				return (-1);
			}
			continue;
		}
		if (str_diffn(line.s, template, line.len - 1)) {
			if (substdio_put(&ssout, line.s, line.len)) {
				strerr_warn3("remove_line: write error: ", fname.s, ": ", &strerr_sys);
				close(fd2);
				unlink(fname.s);
				return (-1);
			}
		} else
			found++;
	}
	if (substdio_flush(&ssout)) {
		strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
		close(fd2);
		unlink(fname.s);
		return (-1);
	}
	close(fd2);
	close(fd1);
	if (rename(fname.s, filename)) {
		strerr_warn5("remove_line: rename: ", fname.s, " --> ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		return (-1);
	}
#ifdef FILE_LOCKING
	delDbLock(lockfd, filename, 1);
#endif
	/*
	 * return 0 = everything went okay, but we didn't find it
	 *        1 = everything went okay and we found a match
	 */
	return (found);
}
