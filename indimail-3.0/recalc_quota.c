/*
 * $Log: recalc_quota.c,v $
 * Revision 1.3  2019-07-26 09:34:57+05:30  Cprogrammer
 * use getDbLock() for locks
 * added FAST_QUOTA env variable to avoid costly disk read for quota calculations
 *
 * Revision 1.2  2019-04-21 16:14:17+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 15:43:49+05:30  Cprogrammer
 * Initial revision
 *
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
#include <strerr.h>
#include <env.h>
#endif
#include "maildir_to_domain.h"
#include "count_dir.h"
#include "check_quota.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "get_assign.h"
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: recalc_quota.c,v 1.3 2019-07-26 09:34:57+05:30 Cprogrammer Exp mbhangui $";
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
	int             fd, i, j;
#ifdef FILE_LOCKING
	int             lockfd;
#endif
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
	if (!force_flag || fast_quota_calc) {
		if (!stat(mdirsizefn.s, &statbuf)) {
			tm = time(0);
			if (fast_quota_calc || ((tm - statbuf.st_mtime) < 43200 && statbuf.st_size <= 8192)) {
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
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(mdirsizefn.s, 1)) == -1) {
		strerr_warn1("recalc_quota: getDbLock: ", &strerr_sys);
		return (-2);
	}
#endif
	if ((fd = open(mdirsizefn.s, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
		if (errno == 2) {
			mail_size = 0;
#ifdef FILE_LOCKING
			delDbLock(lockfd, mdirsizefn.s, 1);
#endif
			return (0);
		}
		strerr_warn3("recalc_quota: ", mdirsizefn.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
	if (fchown(fd, uid, gid) == -1) {
		strerr_warn3("recalc_quota: fchown", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
#ifdef USE_MAILDIRQUOTA
	if (count_limit) {
		strnum1[i = fmt_ulonglong(strnum1, size_limit)] = 0;
		strnum2[j = fmt_ulonglong(strnum1, count_limit)] = 0;
		if (substdio_put(&ssout, strnum1, i) ||
				substdio_put(&ssout, "S,", 2) ||
				substdio_put(&ssout, strnum2, j) || substdio_put(&ssout, "C\n", 2)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, mdirsizefn.s, 1);
#endif
			return (-1);
		}
	} else {
		strnum1[i = fmt_ulonglong(strnum1, size_limit)] = 0;
		if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, "S\n", 2)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, mdirsizefn.s, 1);
#endif
			return (-1);
		}
	}
	if (mail_size || mail_count) {
		strnum1[i = fmt_ulonglong(strnum1, mail_size)] = 0;
		strnum2[j = fmt_ulonglong(strnum1, mail_count)] = 0;
		if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, " ", 1) ||
				substdio_put(&ssout, strnum2, j) || substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, mdirsizefn.s, 1);
#endif
			return (-1);
		}
	}
	if (mailcount)
		*mailcount = mail_count;
#else
	strnum1[i = fmt_ulonglong(strnum1, mail_size)] = 0;
	if (substdio_put(&ssout, strnum1, i) || substdio_put(&ssout, "\n", 1)) {
		strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
#endif
	if (substdio_flush(&ssout)) {
		strerr_warn3("recalc_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, mdirsizefn.s, 1);
#endif
	CurCount = mail_count;
	if (!stralloc_copy(&prevmaildir, &maildir) || !stralloc_0(&prevmaildir))
		die_nomem();
	prevmaildir.len--;
	return (mail_size);
}
