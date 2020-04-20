/*
 * $Log: update_file.c,v $
 * Revision 1.3  2020-03-20 15:12:01+05:30  Cprogrammer
 * BUG Fix. Virtualdomains not created when it doesn't exist
 *
 * Revision 1.2  2019-07-04 00:02:05+05:30  Cprogrammer
 * delete locks on each and every exit
 *
 * Revision 1.1  2019-04-18 08:33:42+05:30  Cprogrammer
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
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <fmt.h>
#include <error.h>
#include <strerr.h>
#include <str.h>
#include <stralloc.h>
#include <getln.h>
#include <substdio.h>
#endif
#include "get_indimailuidgid.h"
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_file.c,v 1.3 2020-03-20 15:12:01+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("update_file: out of memory", 0);
	_exit(111);
}

int
update_file(char *filename, char *update_line, mode_t mode)
{
	struct substdio ssin, ssout;
	int             fd1, fd2, i, found = 0, user_assign = 0, match;
	char            strnum[FMT_ULONG], inbuf[4096], outbuf[4096];
	static stralloc fname = {0}, line = {0};
	struct stat     statbuf;
#ifdef FILE_LOCKING
	int             fd;
#endif

#ifdef FILE_LOCKING
	if ((fd = getDbLock(filename, 1)) == -1) {
		strerr_warn3("update_file: getDbLock(", filename, "): ", &strerr_sys);
		return (-1);
	}
#endif
	strnum[i = fmt_ulong(strnum, getpid())] = 0;
	if (!stralloc_copys(&fname, filename) || !stralloc_append(&fname, ".") ||
			!stralloc_catb(&fname, strnum, i) || !stralloc_0(&fname))
		die_nomem();
	/*- create filename.pid */
	if ((fd2 = open(fname.s, O_CREAT|O_TRUNC|O_WRONLY, mode)) == -1) {
		strerr_warn3("update_file: open: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		return (-1);
	}
	/*- get permission from the original file */
	if (stat(filename, &statbuf)) {
		if (errno != error_noent) {
			strerr_warn3("update_file: stat: ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
		get_indimailuidgid(&statbuf.st_uid, &statbuf.st_gid);
	}
	if (fchown(fd2, statbuf.st_uid, statbuf.st_gid)) {
		strerr_warn3("update_file: fchown: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		close(fd2);
		unlink(fname.s);
		return (-1);
	}
	substdio_fdbuf(&ssout, write, fd2, outbuf, sizeof(outbuf));
	if (access(filename, F_OK)) {
		if (substdio_puts(&ssout, update_line) || substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
		if (substdio_flush(&ssout)) {
			strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		close(fd2);
		if (rename(fname.s, filename)) {
			strerr_warn5("update_file: rename: ", fname.s, " --> ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			unlink(fname.s);
			return (-1);
		}
		unlink(fname.s);
		return (0);
	}
	if ((fd1 = open_read(filename)) == -1) {
		strerr_warn3("update_file: open: ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		close(fd2);
		unlink(fname.s);
		return (-1);
	} else {
		substdio_fdbuf(&ssin, read, fd1, inbuf, sizeof(inbuf));
		for (found = 0;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("update_file: read: ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
				delDbLock(fd, filename, 1);
#endif
				close(fd1);
				close(fd2);
				unlink(fname.s);
				return (-1);
			}
			if (!match && line.len == 0)
				break;
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			if (!str_diffn(line.s, ".\n", 2)) {
				user_assign = 1;
				continue;
			}
			if (!str_diffn(line.s, update_line, line.len - 1)) {
				found = 1;
				if (substdio_puts(&ssout, update_line) || substdio_put(&ssout, "\n", 1)) {
					strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
					delDbLock(fd, filename, 1);
#endif
					close(fd2);
					unlink(fname.s);
					return (-1);
				}
			} else {
				if (substdio_put(&ssout, line.s, line.len)) {
					strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
					delDbLock(fd, filename, 1);
#endif
					close(fd2);
					unlink(fname.s);
					return (-1);
				}
			}
		}
		close(fd1);
	}
	if (!found) { /*- Append */
		if (substdio_puts(&ssout, update_line) || substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
	}
	if (user_assign) {
		if (substdio_put(&ssout, ".\n", 2)) {
			strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			close(fd2);
			unlink(fname.s);
			return (-1);
		}
	}
	if (substdio_flush(&ssout)) {
		strerr_warn3("update_file: write error: ", fname.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		close(fd2);
		unlink(fname.s);
		return (-1);
	}
	close(fd2);
	if (rename(fname.s, filename)) {
		strerr_warn5("update_file: rename: ", fname.s, " --> ", filename, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		unlink(fname.s);
		return (-1);
	}
#ifdef FILE_LOCKING
	delDbLock(fd, filename, 1);
#endif
	return (0);
}
