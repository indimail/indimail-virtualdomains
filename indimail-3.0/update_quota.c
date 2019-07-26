/*
 * $Log: update_quota.c,v $
 * Revision 1.3  2019-07-26 09:38:25+05:30  Cprogrammer
 * use getDbLock() for locking
 *
 * Revision 1.2  2019-04-21 16:14:27+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:33:48+05:30  Cprogrammer
 * Initial revision
 *
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
#ifdef HAVE_SIGNAL_H
#include <signal.h>
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
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_quota.c,v 1.3 2019-07-26 09:38:25+05:30 Cprogrammer Exp mbhangui $";
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
	char            strnum[FMT_ULONG], outbuf[512];
	struct substdio ssout;
	void            (*pstat[3]) ();
	int             fd, i;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

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
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[2] = signal(SIGTSTP, SIG_IGN)) == SIG_ERR)
		return (-1);
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(mdirsizefn.s, 1)) == -1) {
		strerr_warn1("update_quota: getDbLock: ", &strerr_sys);
		return (-2);
	}
#endif
	if ((fd = open(mdirsizefn.s, O_CREAT|O_WRONLY|O_APPEND, S_IWUSR | S_IRUSR)) == -1) {
		strerr_warn3("update_quota: ", mdirsizefn.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
	if (fchown(fd, uid, gid) == -1) {
		strerr_warn3("update_quota: fchown", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
#ifdef USE_MAILDIRQUOTA
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, " 1\n", 3) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
#else
	fprintf(fp, "%"PRIu64"\n", new_size);
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", mdirsizefn.s, ": ", &strerr_sys);
		close(fd);
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
#ifdef FILE_LOCKING
		delDbLock(lockfd, mdirsizefn.s, 1);
#endif
		return (-1);
	}
#endif
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, mdirsizefn.s, 1);
#endif
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	(void) signal(SIGTSTP, pstat[2]);
	return (1);
}
