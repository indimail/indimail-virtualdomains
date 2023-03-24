/*
 * $Log: check_quota.c,v $
 * Revision 1.5  2023-03-20 09:50:46+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2020-10-01 18:22:00+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.3  2019-04-22 23:09:35+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.2  2019-04-21 16:13:29+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:25:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <getln.h>
#include <scan.h>
#include <substdio.h>
#endif
#include "count_dir.h"

#ifndef	lint
static char     sccsid[] = "$Id: check_quota.c,v 1.5 2023-03-20 09:50:46+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("check_quota: out of memory", 0);
	_exit(111);
}

/*
 * Assumes the current working directory is user/Maildir
 *
 * We go off to look at cur and tmp dirs
 * return size of files
 *
 */
#ifdef USE_MAILDIRQUOTA
mdir_t check_quota(char *Maildir, mdir_t *total)
#else
mdir_t check_quota(char *Maildir)
#endif
{
	mdir_t          mail_size;
	static stralloc tmpbuf = {0}, maildir = {0};
	char           *ptr;
	int             fd, i;
	struct flock    fl;
#ifdef USE_MAILDIRQUOTA
	int             match;
	umdir_t         size, count;
	struct substdio ssin;
	static stralloc line = {0};
	static char     inbuf[4096];
#endif

	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9)) {
		*(ptr + 9) = 0;
		maildir.len = str_len(maildir.s);
	}
	fl.l_type   = F_RDLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
#ifdef USE_MAILDIRQUOTA
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
	if (!stralloc_copy(&tmpbuf, &maildir) ||
			!stralloc_catb(&tmpbuf, "/maildirsize", 12) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#else
	if (!stralloc_copy(&tmpbuf, &maildir) ||
			!stralloc_catb(&tmpbuf, "/.current_size", 14) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#endif
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno == ENOENT)
			return (0);
		strerr_warn3("check_quota: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	for (;;) {
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			if (errno == EINTR)
				continue;
			strerr_warn3("check_quota: fcntl(F_SETLKW)", tmpbuf.s, ": ", &strerr_sys);
			(void) close(fd);
			return (-1);
		}
		break;
	}
#ifdef USE_MAILDIRQUOTA
	if (total)
		*total = 0;
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (mail_size = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("check_quota: read: ", tmpbuf.s, ": ", &strerr_sys);
			fl.l_type   = F_UNLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
			if (fcntl(fd, F_SETLK, &fl) == -1)
				strerr_warn3("check_quota: fcntl(F_SETLK)", tmpbuf.s, ": ", &strerr_sys);
			break;
		}
		if (!line.len)
			break;
		if (match) { /*- remove newline */
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		i = str_chr(line.s, ' ');
		if (line.s[i]) {
			scan_ulong(line.s + i + 1, (unsigned long *) &count);
			if (total)
				*total += count;
			line.s[i] = 0;
			scan_ulong(line.s, (unsigned long *) &size);
			mail_size += size;
		}
	} /*- for (mail_size = 0;;) */
	fl.l_type   = F_UNLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	if (fcntl(fd, F_SETLK, &fl) == -1)
		strerr_warn3("check_quota: fcntl(F_SETLK)", tmpbuf.s, ": ", &strerr_sys);
	(void) close(fd);
	if (mail_size < 0 || (total && (*total < 0)))
		mail_size = count_dir(maildir.s, total ? (mdir_t *) total : (mdir_t *) 0);
#else
	fl.l_type   = F_UNLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	if ((mail_size = read(fd, tmpbuf, MAX_BUFF)) == -1) {
		if (fcntl(fd, F_SETLK, &fl) == -1) ; /*- Make compiler happy */
		(void) close(fd);
		return(-1);
	}
	if (fcntl(fd, F_SETLK, &fl) == -1) ; /*- Make compiler happy */
	(void) close(fd);
	tmpbuf[mail_size] = 0;
	scan_ulong(tmpbuf, (unsigned long *) &mail_size);
	if (mail_size < 0)
		mail_size = count_dir(maildir.s, 0);
#endif
	return (mail_size);
}
