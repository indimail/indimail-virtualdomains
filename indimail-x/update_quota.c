/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <substdio.h>
#include <strerr.h>
#endif
#include "get_indimailuidgid.h"
#include "variables.h"
#include "get_assign.h"
#include "maildir_to_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_quota.c,v 1.4 2020-09-21 07:55:21+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("update_quota: out of memory", 0);
	_exit(111);
}

int
update_quota(char *Maildir, mdir_t new_size)
{
	static stralloc mdirsizefn = {0}, maildir = {0};
	char           *ptr;
	uid_t           uid;
	gid_t           gid;
	struct flock    fl = {0};
	char            strnum[FMT_ULONG], outbuf[512];
	struct substdio ssout;
	int             fd, i;

	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
#ifdef USE_MAILDIRQUOTA
	if (!stralloc_copy(&mdirsizefn, &maildir) ||
			!stralloc_catb(&mdirsizefn, "/maildirsize", 12) ||
			!stralloc_0(&mdirsizefn))
		die_nomem();
	if (access(mdirsizefn.s, F_OK))
		return (0);
#else
	if (!stralloc_copy(&mdirsizefn, &maildir) ||
			!stralloc_catb(&mdirsizefn, "/.current_size", 14) ||
			!stralloc_0(&mdirsizefn))
		die_nomem();
#endif
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
	/*- attempt to obtain uid, gid for the maildir */
	if (!(ptr = maildir_to_domain(maildir.s)) || !get_assign(ptr, 0, &uid, &gid)) {
		if (indimailuid == -1 || indimailgid == -1)
			get_indimailuidgid(&indimailuid, &indimailgid);
		uid = indimailuid;
		gid = indimailgid;
	}
	if ((fd = open(mdirsizefn.s, O_CREAT|O_WRONLY|O_APPEND, S_IWUSR | S_IRUSR)) == -1) {
		strerr_warn3("update_quota: ", mdirsizefn.s, ": ", &strerr_sys);
		return (-1);
	}
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	for (;;) {
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			if (errno == EINTR)
				continue;
			strerr_warn3("update_quota: fcntl: F_SETLKW", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		break;
	}
	fl.l_type = F_UNLCK;
	if (fchown(fd, uid, gid) == -1) {
		strerr_warn3("update_quota: fchown", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("update_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
#ifdef USE_MAILDIRQUOTA
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, " 1\n", 3) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("update_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
#else
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("update_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
#endif
	if (fcntl(fd, F_SETLK, &fl) == -1)
		strerr_warn3("update_quota: fcntl: F_SETLK", mdirsizefn.s, ": ", &strerr_sys);
	close(fd);
	return (1);
}
/*
 * $Log: update_quota.c,v $
 * Revision 1.4  2020-09-21 07:55:21+05:30  Cprogrammer
 * fixed incorrect initialization of struct flock
 *
 * Revision 1.3  2019-07-26 22:19:43+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.2  2019-04-21 16:14:27+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:33:48+05:30  Cprogrammer
 * Initial revision
 *
 */
