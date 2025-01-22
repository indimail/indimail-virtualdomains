/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <substdio.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <strerr.h>
#include <env.h>
#endif
#include "maildir_to_domain.h"
#include "count_dir.h"
#include "check_quota.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "get_assign.h"

#ifndef	lint
static char     sccsid[] = "$Id: recalc_quota.c,v 1.7 2020-10-12 12:41:07+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("recalc_quota: out of memory", 0);
	_exit(111);
}

#ifdef USE_MAILDIRQUOTA
mdir_t recalc_quota(char *Maildir, mdir_t *mailcount, mdir_t size_limit, mdir_t count_limit, int force_flag)
#else
mdir_t recalc_quota(char *Maildir, int force_flag)
#endif
{
	static mdir_t   mail_size, mail_count;
	struct flock    fl = {0};
	int             fd, i, j;
	char            strnum1[FMT_ULONG], outbuf[512];
	struct substdio ssout;
#ifdef USE_MAILDIRQUOTA
	char            strnum2[FMT_ULONG];
#endif
	static stralloc prevmaildir = {0}, mdirsizefn = {0}, maildir = {0};
	char           *ptr;
	struct stat     statbuf;
	time_t          tm;
	uid_t           uid;
	gid_t           gid;
	int             fast_quota_calc;
	unsigned long   max_age = 43200, max_size = 8192;

	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9)) {
		*(ptr + 9) = 0;
		maildir.len = str_len(maildir.s);
	}
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
#ifdef USE_MAILDIRQUOTA
	if (mailcount)
		*mailcount = 0;
	if (!stralloc_copy(&mdirsizefn, &maildir) ||
			!stralloc_catb(&mdirsizefn, "/maildirsize", 12) ||
			!stralloc_0(&mdirsizefn))
		die_nomem();
#else
	if (!stralloc_copy(&mdirsizefn, &maildir) ||
			!stralloc_catb(&mdirsizefn, "/.current_size", 14) ||
			!stralloc_0(&mdirsizefn))
		die_nomem();
#endif
	fast_quota_calc = env_get("FAST_QUOTA") ? 1 : 0;
	if ((ptr = env_get("MAILDIRSIZE_MAX_AGE")))
		scan_ulong(ptr, &max_age);
	if ((ptr = env_get("MAILDIRSIZE_MAX_SIZE")))
		scan_ulong(ptr, &max_size);
	if (!force_flag || fast_quota_calc) {
		if (!stat(mdirsizefn.s, &statbuf)) {
			tm = time(0);
			if (fast_quota_calc || ((tm - statbuf.st_mtime) < max_age && statbuf.st_size <= max_size)) {
#ifdef USE_MAILDIRQUOTA
				if (!mail_size || !mail_count)
					mail_size = check_quota(maildir.s, &mail_count);
				if (mailcount)
					*mailcount = mail_count;
#else
				if (!mail_size)
					mail_size = check_quota(maildir.s);
#endif
				return (mail_size);
			}
		}
	}
	if (force_flag != 2) { /*- Cache it */
		if (prevmaildir.len && !str_diffn(maildir.s, prevmaildir.s, prevmaildir.len + 1))
			return (mail_size);
	}
	if (!(ptr = maildir_to_domain(maildir.s)) || !get_assign(ptr, 0, &uid, &gid)) {
		if (indimailuid == -1 || indimailgid == -1)
			get_indimailuidgid(&indimailuid, &indimailgid);
		uid = indimailuid;
		gid = indimailgid;
	}
	/*- recursive function, can take time for a large Maildir */
	mail_size = count_dir(maildir.s, &mail_count);
	if ((fd = open(mdirsizefn.s, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
		if (errno == 2) {
			mail_size = 0;
			return (0);
		}
		strerr_warn3("recalc_quota: ", mdirsizefn.s, ": ", &strerr_sys);
		return (-1);
	}
	fl.l_pid = getpid();
	/*- lock the file */
	for (;;) {
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			if (errno == EINTR)
				continue;
			strerr_warn3("recalc_quota: fcntl(F_SETLKW)", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		break;
	}
	fl.l_type = F_UNLCK;
	if (fchown(fd, uid, gid) == -1) {
		strerr_warn3("recalc_quota: fchown", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
#ifdef USE_MAILDIRQUOTA
	if (count_limit) {
		strnum1[i = fmt_ulonglong(strnum1, size_limit)] = 0;
		strnum2[j = fmt_ulonglong(strnum2, count_limit)] = 0;
		if (substdio_put(&ssout, strnum1, i) ||
				substdio_put(&ssout, "S,", 2) ||
				substdio_put(&ssout, strnum2, j) || substdio_put(&ssout, "C\n", 2)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			if (fcntl(fd, F_SETLK, &fl) == -1)
				strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
	} else {
		strnum1[i = fmt_ulonglong(strnum1, size_limit)] = 0;
		if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, "S\n", 2)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			if (fcntl(fd, F_SETLK, &fl) == -1)
				strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
	}
	if (mail_size || mail_count) {
		strnum1[i = fmt_ulonglong(strnum1, mail_size)] = 0;
		strnum2[j = fmt_ulonglong(strnum2, mail_count)] = 0;
		if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, " ", 1) ||
				substdio_put(&ssout, strnum2, j) || substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			if (fcntl(fd, F_SETLK, &fl) == -1)
				strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
	}
	if (mailcount)
		*mailcount = mail_count;
#else
	strnum1[i = fmt_ulonglong(strnum1, mail_size)] = 0;
	if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, "\n", 1)) {
		strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
#endif
	if (substdio_flush(&ssout)) {
		strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		if (fcntl(fd, F_SETLK, &fl) == -1)
			strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
	if (fcntl(fd, F_SETLK, &fl) == -1)
		strerr_warn3("recalc_quota: fcntl(F_SETLK)", mdirsizefn.s, ": ", &strerr_sys);
	close(fd);
	CurCount = mail_count;
	if (!stralloc_copy(&prevmaildir, &maildir) || !stralloc_0(&prevmaildir))
		die_nomem();
	prevmaildir.len--;
	return (mail_size);
}
/*
 * $Log: recalc_quota.c,v $
 * Revision 1.7  2020-10-12 12:41:07+05:30  Cprogrammer
 * maildirquota problem: strnum2 variable wasn't getting set
 *
 * Revision 1.6  2020-09-21 07:55:11+05:30  Cprogrammer
 * fixed incorrect initialization of struct flock
 *
 * Revision 1.5  2019-07-29 14:02:12+05:30  Cprogrammer
 * added scan.h for scan_ulong()
 *
 * Revision 1.4  2019-07-27 11:26:53+05:30  Cprogrammer
 * added FAST_QUOTA, MAILDIRSIZE_MAX_SIZE, MALIDISIZE_MAX_AGE variables for improving locking on NFS mounted Maildirs
 *
 * Revision 1.3  2019-07-26 21:54:40+05:30  Cprogrammer
 * added FAST_QUOTA env variable to avoid costly disk read for quota calculations
 *
 * Revision 1.2  2019-04-21 16:14:17+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 15:43:49+05:30  Cprogrammer
 * Initial revision
 *
 */
